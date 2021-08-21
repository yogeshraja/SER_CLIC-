#include <bits/stdc++.h>
#include <condition_variable>
#include "fileQueue.h"
#define MAX_THREAD_COUNT 4

#ifdef _WIN32
#include<thread>
#endif

#ifdef __linux__
#include <thread>
#endif

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
        cout << "\n[+] File Processing --- remaining files to process: " << fpQueue.size() << endl;
        mtx.unlock();
        return temp;
    }
    bool addFile(fileQueue *fq)
    {
        mtx.lock();
        fpQueue.push(fq);
        cout << "\n[+] File added --- remaining files to process: " << fpQueue.size() << endl;
        mtx.unlock();
        return true;
    }

    bool isEmpty()
    {
        return fpQueue.empty();
    }

    filePool()
    {
        cout << "\n[+] filepool initialized " << endl;
    }

    void fileProcessor(filePool *fp, fileQueue *fq)
    {
        string data;
        char delimiter = fq->delimiter;
        int columNo = fq->columno;
        fstream fs(fq->filepath, ios::in);
        int i = 0;
        while (getline(fs, data, delimiter))
        {
            i++;
            if (i == columNo)
            {
                fs.trunc;
                fs << data;
            }
        }
        fs.close();

        if (i != columNo)
            cerr << "\n[-] Processing the file " << fq->filename << " failed" << endl;
        else
        {
            fstream ofs(fq->filepath, ios::out | ios::trunc);
            ofs << data;
            ofs.close();
            cout << "\n[+] file " << fq->filename << " processed successfully" << endl;
        }
        fp->threadCount--;
    }
};