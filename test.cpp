#include <bits/stdc++.h>
#include "fileQueue.h"
using namespace std;

///home/yogesh/PERSONAL/MOVIES/ENGLISH/Avengers Age of Ultron (2015).mkv

int main()
{
    queue<fileQueue *> fq;
    fq.push(new fileQueue("send.txt"));
    fq.push(new fileQueue("Send.txt"));
    fq.push(new fileQueue("c:\\windows\\winhelp.exe"));
    fq.push(new fileQueue("/home/yogesh/PERSONAL/MOVIES/ENGLISH/Avengers Age of Ultron (2015).mkv"));
}