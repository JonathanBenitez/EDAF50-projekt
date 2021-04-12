#ifndef COMMAND_H
#define COMMAND_H
#include<string>
#include<vector>

using std::vector;
using std::string;

class Command {
public:
    Command(int n, string s): info{s},i{n}{};
    int getInt(){return i;};
    string getInfo(){return info;};
private:
    string info;
    int i;
};

#endif

