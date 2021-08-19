#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/stat.h>
#include "headers/progressbar.h"
#include "headers/filePool.h"
#define PORT 8989
#define W_PORT "8989"
#define T_MAX 10
#define SER_FOLDER "Received"
using namespace std;

//Windows specific includes
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "Mswsock")
#pragma comment(lib, "AdvApi32")

typedef SOCKET nsock;
typedef struct addrinfo sockaddrinfo;
#endif

//Linux specific includes
#ifdef __linux__
#include <arpa/inet.h>
typedef int nsock;
typedef sockaddr_in sockaddrinfo;
#endif

//Initializing poolHandler object for creating threads for file processing
filePool *poolHandler = new filePool();

//Function to read data from the socket
bool readdata(nsock sock, void *buf, int buflen)
{
    char *pbuf = (char *)buf;
    while (buflen > 0)
    {
        char readbuf[1024];
        int num = recv(sock, pbuf, buflen, 0);
        if (num == -1)
        {
            cerr << "\n[-]Error while receiving file...";
            return false;
        }
        else if (num == 0)
            return false;

        pbuf += num;
        buflen -= num;
    }

    return true;
}

//Function to call the file processor from the pool handler
void callFileProcessor(filePool *fp, fileQueue *fq)
{
    poolHandler->fileProcessor(fp, fq);
}

// Function to create threads of file processor function
void ProcessFiles()
{
    cout << "\n[+] Files processor initiated... " << endl;
    while (true)
    {
        if (poolHandler->threadCount <= T_MAX && !poolHandler->isEmpty())
        {
            fileQueue *tempfq = poolHandler->getFile();
            poolHandler->threadCount++;
            thread t(callFileProcessor, poolHandler, tempfq);
            t.detach();
        }
    }
}

bool readlong(nsock sock, long *value)
{
    if (!readdata(sock, value, sizeof(value)))
        return false;
    *value = ntohl(*value);
    return true;
}

bool readfile(nsock sock, FILE *f)
{
    long filesize;
    if (!readlong(sock, &filesize))
        return false;
    if (filesize > 0)
    {
        char buffer[1024];
        do
        {
            int num = min<long>(filesize, sizeof(buffer));
            if (!readdata(sock, buffer, num))
                return false;
            int offset = 0;
            do
            {
                size_t written = fwrite(&buffer[offset], 1, num - offset, f);
                if (written < 1)
                    return false;
                offset += written;
            } while (offset < num);
            filesize -= num;
        } while (filesize > 0);
    }
    return true;
}

void receiveFile(nsock sockfd)
{
#ifdef __linux__
    mkdir(SER_FOLDER, 0777);
#endif
#ifdef _WIN32
    mkdir(SER_FOLDER);
#endif
    long fnameSize;
    char filename[1024];
    if (!readlong(sockfd, &fnameSize))
    {
        cerr << "\n[-] Filename length reception failed";
        return;
    }
    cout << "\n file name length : " << fnameSize;
    if (!readdata(sockfd, filename, fnameSize))
    {
        cerr << "\n[-] Filename reception failed";
        return;
    }
    char delim;
    long colno;
    string serverFolder = SER_FOLDER;
    filename[fnameSize] = '\0';
    cout << "\n[+] Receiving " << filename;

    string fname = serverFolder + "/" + string(filename);

    FILE *filehandle = fopen(fname.c_str(), "wb");
    if (filehandle != NULL)
    {
        bool ok = readfile(sockfd, filehandle);
        // fclose(filehandle);
        if (!ok)
            remove(filename);
    }
    string rfilename = string(filename);
    if (!readlong(sockfd, (long *)&delim))
    {
        return;
    }
    if (!readlong(sockfd, &colno))
    {
        return;
    }
    cout << "\n[+] Delimiter is " << delim;
    cout << "\n[+] Colum number is " << colno << endl;
    fileQueue *fq = new fileQueue(rfilename, filehandle, delim, colno);
    fq->filepath = fname;
    poolHandler->addFile(fq);
}

void clientThread(nsock newsock)
{
    long fileCount = 0;
    if (!readlong(newsock, &fileCount))
    {
        cerr << "\n[-] Filecount reception failed";
        cerr << "\n[-] Disconnecting the client" << endl;
        return;
    }
    cout << "\n[+] Receiving " << fileCount << " files" << endl;
    while (fileCount--)
        receiveFile(newsock);
    close(newsock);
}

int main(int argc, char **args)
{

    nsock sockfd;
    sockaddrinfo sockdesc;
#ifdef _WIN32
    WSADATA WSAData;
    struct addrinfo *result = NULL;
    if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
    {
        cerr << "\n[-] WSAStatup failed ";
        exit(-1);
    }
    ZeroMemory(&sockdesc, sizeof(sockdesc));
    sockdesc.ai_family = AF_INET;
    sockdesc.ai_socktype = SOCK_STREAM;
    sockdesc.ai_protocol = IPPROTO_TCP;
    sockdesc.ai_flags = AI_PASSIVE;

    if ((getaddrinfo(NULL, W_PORT, &sockdesc, &result)) != 0)
    {
        cerr << "\n[-] getaddrinfo failed ";
        WSACleanup();
        return 1;
    }
    cout << "\n[+] address resolution successful ";
    if ((sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
    {
        cerr << "\n[-] socket creation failed with error " << WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    cout << "\n[+] socket creation successful sock_fd: " << sockfd;
    freeaddrinfo(result);

    if((bind(sockfd,result->ai_addr,(int)result->ai_addrlen))==SOCKET_ERROR){
        cerr<<"\n[-] bind failed with error:"<<WSAGetLastError();
        freeaddrinfo(result);
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    cout << "\n[+] bound to port " << W_PORT;

    if ((listen(sockfd, SOMAXCONN)) == SOCKET_ERROR)
    {
        cerr << "\n[-] listening to the port failed ";
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    cout << "\n[+] listening on port " << W_PORT;
    thread fileprocessorthread(ProcessFiles);
    while (true)
    {
        nsock newsock;
        sockaddrinfo newaddr;
        int newaddrlen = sizeof(newaddr);
        newsock = accept(sockfd, (struct sockaddr *)&newaddr, &newaddrlen);
        if (newsock == INVALID_SOCKET)
        {
            cerr << "[-] Connection with the client failed \n";
        }
        else
        {
            cout << "\n[+] Connected to the client sock_fd:" << newsock << endl;
            thread temp(clientThread, newsock);
            temp.detach();
        }
    }
#endif

#ifdef __linux__
    //create a socket and store the socket file descrptor
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //exit on scoket creating failure
    if (sockfd == -1)
    {
        cerr << "[-] socket creation failed \n";
        exit(-1);
    }
    cout << "[+] socket creation successful sock_fd:" << sockfd << endl;
    sockdesc.sin_family = AF_INET;
    sockdesc.sin_port = htons(PORT);
    sockdesc.sin_addr.s_addr = INADDR_ANY;

    //remove the padding in the internal sock descriptor
    memset(sockdesc.sin_zero, 0, sizeof(sockdesc.sin_zero));

    //bind the socket to an interface
    if (bind(sockfd, (struct sockaddr *)&sockdesc, sizeof(sockdesc)) == -1)
    {
        cerr << "[-] bind to port " << PORT << " failed\n";
        exit(-2);
    }
    cout << "[+] bind to port " << PORT << " successful\n";
    if (listen(sockfd, 3) == -1)
    {
        cerr << "[-] set port to listen failed \n";
        exit(-3);
    }
    cout << "[+] listening on port " << PORT << endl;
    thread fileprocessorthread(ProcessFiles);
    while (true)
    {
        nsock newsock;
        sockaddrinfo newaddr;
        int newaddrlen = sizeof(newaddr);
        newsock = accept(sockfd, (struct sockaddr *)&newaddr, &newaddrlen);
        if (newsock == -1)
        {
            cerr << "[-] Connection with the client failed \n";
        }
        cout << "[+] Connected to the client sock_fd:" << newsock << endl;
        if (newsock != -1)
        {
            thread temp(clientThread, newsock);
            temp.detach();
        }
    }
#endif
    fileprocessorthread.join();
    close(sockfd);
}
