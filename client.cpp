#include <bits/stdc++.h>
#include <unistd.h>
#include "headers/progressbar.h"
#include "headers/fileQueue.h"
#define PORT 8989
#define W_PORT "8989"
using namespace std;

//Windows specific includes
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ws2tcpip.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "Mswsock")
#pragma comment(lib, "AdvApi32")

typedef SOCKET nsock;
typedef struct addrinfo sockaddrinfo;
#endif

//linux specific includes
#ifdef __linux__
#include <arpa/inet.h>
typedef int nsock;
typedef sockaddr_in sockaddrinfo;
#endif

queue<fileQueue *> fq;

bool senddata(int sock, void *buf, int buflen)
{
    char *pbuf = (char *)buf;

    while (buflen > 0)
    {
        int num = send(sock, (const char *)pbuf, buflen, 0);
        if (num == -1)
        {
            cerr << "\n[-]Error occured while transmitting" << endl;
            return false;
        }

        pbuf += num;
        buflen -= num;
    }

    return true;
}

bool sendlong(int sock, long value)
{
    value = htonl(value);
    return senddata(sock, &value, sizeof(value));
}

bool sendfile(int sock, fileQueue *fileData)
{
    FILE *f = fileData->fp;
    long filesize = fileData->filesize;
    string filename = fileData->filename;
    long fnameLength = filename.size();
    long ptrack = filesize;
    if (!sendlong(sock, fnameLength))
        return false;
    if (!senddata(sock, (void *)filename.c_str(), fnameLength))
    {
        return false;
    }
    if (filesize == EOF)
        return false;
    if (!sendlong(sock, filesize))
        return false;
    if (filesize > 0)
    {
        char buffer[1024];
        do
        {
            size_t num = min<long>(filesize, sizeof(buffer));
            num = fread(buffer, 1, num, f);
            if (num < 1)
                return false;
            if (!senddata(sock, buffer, num))
                return false;
            filesize -= num;
            printProgress(ptrack, filesize);
        } while (filesize > 0);
    }
    if (!sendlong(sock, fileData->delimiter))
    {
        return false;
    }
    if (!sendlong(sock, fileData->columno))
    {
        return false;
    }
    return true;
}

void openFile()
{
    string file;
    char choice;
    bool flag = true;
    FILE *filehandle;
    cout << "\nPlease enter the filename or the path to the file: ";
    getline(cin, file);
    fileQueue *obj = new fileQueue(file);
    if (obj->fp == NULL)
    {
        cout << "\n[+] Would you like to retry[Y/n]: ";
        if (toupper(choice = getchar()) == 'Y')
        {
            getchar();
            openFile();
        }
        else
        {
            flag = false;
            cout << "\n[+] Starting " << fq.size() << " transfer(s)...";
            return;
        }
    }
    else
    {
        cout << "\n[+] Please enter the delimiter: ";
        obj->delimiter = getchar();
        cout << "\n[+] The delimiter is " << obj->delimiter;
        cout << "\n[+] Please enter the column number: ";
        cin >> obj->columno;
        cout << "\n[+] The column number is " << obj->columno;
        getchar();
        fq.push(obj);
    }
    cout << "\n[+] Would you like to add more files[Y/n]: ";
    if (toupper(choice = getchar()) == 'Y' && flag)
    {
        getchar();
        openFile();
    }
    else
    {
        cout << "\n[+] Starting " << fq.size() << " transfer(s)..." << endl;
    }
}

int main(int argc, char **args)
{
    nsock sockfd;
    sockaddrinfo sockdesc;
    if (argc != 2)
    {
        cerr << "\n[-] Usage: " << args[0] << " server-name/ip-address";
        exit(-1);
    }
#ifdef _WIN32
    WSADATA WSAData;
    sockaddrinfo *result = NULL, *ptr = NULL;
    if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
    {
        cerr << "\n[-] WSAStartup failed";
        exit(-1);
    }
    ZeroMemory(&sockdesc, sizeof(sockdesc));
    sockdesc.ai_family = AF_INET;
    sockdesc.ai_socktype = SOCK_STREAM;
    sockdesc.ai_protocol = IPPROTO_TCP;

    if ((getaddrinfo(args[1], W_PORT, &sockdesc, &result)))
    {
        cerr << "\n[-] Failed to resolve server error-code: " << WSAGetLastError();
        WSACleanup();
        return 1;
    }
    //Connect to one of the resolved addresses
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        //create a socket
        if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == INVALID_SOCKET)
        {
            cerr << "\n[-] socket creation failed with error " << WSAGetLastError();
            WSACleanup();
            return 1;
        }
        //connect to the server
        if ((connect(sockfd, ptr->ai_addr, (int)ptr->ai_addrlen)) == SOCKET_ERROR)
        {
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (sockfd == -1)
    {
        cerr << "\n[-] unable to connect to the server ";
        WSACleanup();
        return 1;
    }
    cout << "\n[+] socket creation successful sock_fd:" << sockfd;
#endif
#ifdef __linux__
    //create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        cerr << "[-] socket creation successful sock_fd:" << sockfd << endl;
        exit(-1);
    }
    cout << "[+] socket creation successfull sock_fd:" << sockfd << endl;
    sockdesc.sin_family = AF_INET;
    sockdesc.sin_port = htons(PORT);
    sockdesc.sin_addr.s_addr = INADDR_ANY;
    memset(sockdesc.sin_zero, 0, sizeof(sockdesc.sin_zero));
    long sockdescsize = sizeof(sockdesc);

    if ((connect(sockfd, (struct sockaddr *)&sockdesc, sockdescsize)) == -1)
    {
        cerr << "[-] connection to the server failed \n";
        exit(-2);
    }
#endif
    cout << "[+] connected to the server on port " << PORT << endl;
    openFile();
    long filecount = fq.size();
    if (!sendlong(sockfd, filecount))
    {
        cerr << "\n[-] File count transfer failed";
    }
    while (!fq.empty())
    {
        cout << "\n[+] Transferring " << fq.front()->filename << endl;
        cout << (sendfile(sockfd, fq.front()) ? "\nTransfer successful" : "\nTransfer failed") << endl;
        removeFileFromQueue(fq);
    }

    close(sockfd);
}