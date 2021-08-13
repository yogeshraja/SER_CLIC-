#include <bits/stdc++.h>
#include <threads.h>
#include <condition_variable>
#include "fileQueue.h"
#define MAX_THREAD_COUNT 4
using namespace std;

class filePool
{
    queue<fileQueue *> fpQueue;
    mutex mtx;
    condition_variable cv;

public:
    int threadCount = 0;

    fileQueue *getFile()
    {
        mtx.lock();
        fileQueue *temp = fpQueue.front();
        fclose(temp->fp);
        fpQueue.pop();
        cout << "\n[+] File Processing --- remaining files to process: " << fpQueue.size()<<endl;
        mtx.unlock();
        return temp;
    }
    bool addFile(fileQueue *fq){
        mtx.lock();
        fpQueue.push(fq);
        cout << "\n[+] File added --- remaining files to process: " << fpQueue.size()<<endl;
        mtx.unlock();
        return true;
    }

    bool isEmpty(){
        return fpQueue.empty();
    }

    filePool(){
        cout << "\n[+] filepool initialized " << endl;
    }

    void fileProcessor(filePool *fp, fileQueue *fq){
        string data;
        FILE *filehandle=fq->fp;
        char delimiter = fq->delimiter;
        int columNo = fq->columno;
        fstream fs(fq->filepath,ios::in);
        int i = 0;
        while (getline(fs,data,delimiter))
        {
            i++;
            if(i==columNo){
                fs.trunc;
                fs << data;
            }
        }
        if(i!=columNo)
            cerr << "\n[-] Processing the file " << fq->filename << " failed";
        fs.close();
        fstream ofs(fq->filepath, ios::out|ios::trunc);
        ofs<<data;
        ofs.close();
        fp->threadCount --;
        cout << "\n[+] file " << fq->filename << "Processed successfully";
        }
};