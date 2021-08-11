#include <bits/stdc++.h>
#include<unistd.h>
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(size_t acSize, size_t remSize)
{
    float percentage = 1.0 - ((double)remSize / acSize);
    int val = (int)(percentage * 100);
    int lpad = (int)(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}

void updateProgress(size_t acSize,size_t remSize){
    std::cout.flush();
    float progress = 1.0 - ((float)remSize / acSize);
    int barWidth = 70;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i <= barWidth; ++i)
    {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "\r";
    // std::cout << std::endl;
}
