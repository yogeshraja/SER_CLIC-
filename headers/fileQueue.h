#include<bits/stdc++.h>
using namespace std;

class fileQueue{
    private:
        void stripFileName()
        {
            size_t pos;
            if ((pos = filename.rfind("/")) != string::npos || (pos = filename.rfind("\\")) != string::npos)
            {
                filename = filename.substr(pos + 1);
            }
        }

        string detFileSize(FILE *fp)
        {
            fseek(fp, 0L, SEEK_END);
            filesize = ftell(fp);
            rewind(fp);
            return to_string(filesize / (1024 * 1024)) + " MB";
        }

    public:
        FILE *fp;
        string filename;
        string filepath;
        double filesize;

        fileQueue(string fname){
            filename = fname;
            filepath = fname;
            stripFileName();
            if((fp=fopen(fname.c_str(),"rb"))==NULL){
                cerr << "\n[-] File: " << filename << " Not found";
            }
            else{
                cout << "\n[+] " << filename << ": " << detFileSize(fp);
            }
        }
};

void removeFileFromQueue(queue<fileQueue *> &fq){
    fclose(fq.front()->fp);
    delete fq.front();
    fq.pop();
}