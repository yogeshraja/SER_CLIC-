#include <bits/stdc++.h>
#include <arpa/inet.h>
#include<unistd.h>
#include"progressbar.h"
#define PORT 8989
using namespace std;

// int loadSendBuffer(){

// }

// u_int64_t sendFile(int sockfd,string filename){
    
// }

bool senddata(int sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *)buf;

    while (buflen > 0)
    {
        int num = send(sock, pbuf, buflen, 0);
        if (num == -1)
        {
            cerr << "\n[-]Error occured while transmitting";
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

bool sendfile(int sock, FILE *f)
{
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    long ptrack = filesize;
    rewind(f);
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
    return true;
}

FILE* openFile(){
    string file;
    FILE *filehandle;
    cout << "\nPlease enter the filename or the path to the file: ";
    getline(cin, file);
    if((filehandle = fopen(file.c_str(), "rb"))==NULL){
        cerr << "[-]file not found at the specified location\n";
        cout << "[+]Would you like to retry[Y/n]: ";
        if(toupper(getchar())=='Y')
            return openFile();
        else{
            cout << "[+]exiting the program...\n";
            exit(-1);
        }
    }
    return filehandle;
}

int main()
{
    int sockfd;
    sockaddr_in sockdesc;
    //create a socket
    if((sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1){
        cerr << "[-]socket creation successful sock_fd:" << sockfd << endl;
        exit(-1);
    }
    cout<<"[+]socket creation successfull sock_fd:"<<sockfd<<endl;
    sockdesc.sin_family = AF_INET;
    sockdesc.sin_port=htons(PORT);
    sockdesc.sin_addr.s_addr = INADDR_ANY;
    memset(sockdesc.sin_zero, 0, sizeof(sockdesc.sin_zero));
    socklen_t sockdescsize=sizeof(sockdesc);

    if((connect(sockfd,(struct sockaddr *)&sockdesc,sockdescsize))==-1){
        cerr << "[-]connection to the server failed \n";
        exit(-2);
    }
    cout << "[+]Connected to the server on port " << PORT << endl;
    FILE *fp = openFile();
    cout<<(sendfile(sockfd, fp)?"\nTransfer successful":"\nTransfer failed");
    fclose(fp);
    close(sockfd);
}