#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include "headers/progressbar.h"
#include "headers/filePool.h"
#define PORT 8989
#define T_MAX 10
#define SER_FOLDER "Received"
using namespace std;

#ifdef _WIN32
typedef SOCKET nsock;
#endif

#ifdef __linux__
typedef int nsock;
#endif

filePool *poolHandler=new filePool();
bool readdata(nsock sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *)buf;

    while (buflen > 0)
    {
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

void callFileProcessor(filePool *fp,fileQueue *fq){
    poolHandler->fileProcessor(fp,fq);
}

void ProcessFiles(){
    cout << "\n[+] Files processor initiated... "<<endl;
    while(true){
        if(poolHandler->threadCount<=T_MAX && !poolHandler->isEmpty()){
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
    mkdir(SER_FOLDER, 0777);
    long fnameSize;
    char filename[1024];
    if (!readlong(sockfd, &fnameSize))
    {
        cerr << "\n[-] Filename reception failed";
        return;
    }

    if (!readdata(sockfd, filename, fnameSize))
    {
        cerr << "\n[-] Filename reception failed";
        return;
    }
    char delim;
    long colno;
    string serverFolder = SER_FOLDER;
    filename[fnameSize] = '\0';
    cout << "\n[+] Receiving " << filename << endl;

    string fname = serverFolder + "/" + string(filename);

    FILE *filehandle = fopen(fname.c_str(), "wb");
    if (filehandle != NULL)
    {
        bool ok = readfile(sockfd, filehandle);
        // fclose(filehandle);
        if (ok)
        {
            // use file as needed...
        }
        else
            remove(filename);
    }
    if (!readlong(sockfd, (long *)&delim))
    {
        return;
    }
    if (!readlong(sockfd, &colno))
    {
        return;
    }
    cout << "\n[+] Delimiter is " << delim << endl;
    cout << "\n[+] Colum number is " << colno << endl;
    fileQueue *fq = new fileQueue(string(filename), filehandle, delim, colno);
    fq->filepath = fname;
    poolHandler->addFile(fq);
}

void clientThread(nsock newsock)
{
    long fileCount;
    if (!readlong(newsock, &fileCount))
    {
        cerr << "\n[-] Filecount reception failed";
    }
    cout << "\n[+] Receiving " << fileCount << " files" << endl;
    while (fileCount--)
        receiveFile(newsock);
    close(newsock);
}

int main(int argc, char **args)
{
    nsock sockfd;
    sockaddr_in sockdesc;

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
    vector<thread> tPool;
    int tSize = 0;
    thread fileprocessorthread(ProcessFiles);
    while (true)
    {
        nsock newsock;
        sockaddr_in newaddr;
        socklen_t newaddrlen = sizeof(newaddr);
        newsock = accept(sockfd, (struct sockaddr *)&newaddr, &newaddrlen);
#ifdef __linux__
        if (newsock == -1)
        {
            cerr << "[-] Connection with the client failed \n";
        }
#endif
        cout << "[+] Connected to the client sock_fd:" << newsock << endl;
        if (newsock != -1)
            tPool.push_back(thread(clientThread, newsock));
        // close(newsock);
        if (T_MAX <= tSize)
            break;
    }
    while (--tSize)
    {
        tPool.at(tSize).join();
    }
    fileprocessorthread.join();
    close(sockfd);
}
