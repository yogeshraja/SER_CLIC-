#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8989

using namespace std;

bool readdata(int sock, void *buf, int buflen)
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

bool readlong(int sock, long *value)
{
    if (!readdata(sock, value, sizeof(value)))
        return false;
    *value = ntohl(*value);
    return true;
}

bool readfile(int sock, FILE *f)
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

void receiveFile(int sockfd)
{
    long fnameSize;
    char filename[1024];
    if (!readlong(sockfd, &fnameSize)){
        cerr << "\n[-] Filename reception failed";
        return;
    }
    if (!readdata(sockfd, filename, fnameSize)){
        cerr << "\n[-] Filename reception failed";
        return ;
    }
    FILE *filehandle = fopen(filename, "wb");
    if (filehandle != NULL)
    {
        bool ok = readfile(sockfd, filehandle);
        fclose(filehandle);
        if (ok)
        {
            // use file as needed...
        }
        else
            remove(filename);
    }
}

int main()
{
    int sockfd;
    sockaddr_in sockdesc;

    //create a socket and store the socket file descrptor
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //exit on scoket creating failure
    if (sockfd == -1)
    {
        cerr << "[-]socket creation failed \n";
        exit(-1);
    }
    cout << "[+]socket creation successful sock_fd:" << sockfd << endl;
    sockdesc.sin_family = AF_INET;
    sockdesc.sin_port = htons(PORT);
    sockdesc.sin_addr.s_addr = INADDR_ANY;

    //remove the padding in the internal sock descriptor
    memset(sockdesc.sin_zero, 0, sizeof(sockdesc.sin_zero));

    //bind the socket to an interface
    if (bind(sockfd, (struct sockaddr *)&sockdesc, sizeof(sockdesc)) == -1)
    {
        cerr << "[-]bind to port " << PORT << " failed\n";
        exit(-2);
    }
    cout << "[+]bind to port " << PORT << " successful\n";
    if (listen(sockfd, 3) == -1)
    {
        cerr << "[-]set port to listen failed \n";
        exit(-3);
    }
    cout << "[+]listening on port " << PORT << endl;
    socklen_t newsock;
    sockaddr_in newaddr;
    socklen_t newaddrlen = sizeof(newaddr);
    newsock = accept(sockfd, (struct sockaddr *)&newaddr, &newaddrlen);
    if (newsock == -1)
    {
        cerr << "[-]Connection with the client failed \n";
        exit(-4);
    }
    cout << "[+]Connected to the client sock_fd:" << newsock << endl;
    receiveFile(newsock);
    close(newsock);
    close(sockfd);
}
