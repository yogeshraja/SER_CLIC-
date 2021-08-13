#include <bits/stdc++.h>
#include "headers/filePool.h"
using namespace std;

///home/yogesh/PERSONAL/MOVIES/ENGLISH/Avengers Age of Ultron (2015).mkv
///home/yogesh/PERSONAL/MOVIES/ENGLISH/mr and mrs smith.mkv

int main()
{
    fileQueue *fq = new fileQueue("send.txt",',',4);
    fq->filepath = "send.txt";
    filePool *fp = new filePool();
    fp->fileProcessor(fp,fq);
}