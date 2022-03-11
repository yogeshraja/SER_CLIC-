#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

string exec(string command)
{
    string result = "";
    char buffer[128];

    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        return "\n[-] IP fetch failed";
    }

    while (!feof(pipe))
    {

        if (fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }
    pclose(pipe);
    return result;
}