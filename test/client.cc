#include <iomanip>
#include <utility>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include "messageHandler.h"

using std::cout;
using std::endl;
using std::cin;
using std::string;
using std::vector;

std::vector<std::string> flags{"NEW", "DEL", "READ"};
std::vector<std::string> commands{"group","ls","article", "lsarticles"};

void addArticleInfo(vector<string>& v) {
    string t;
    cout << "Enter the name of the article:" << endl;
    std::getline(cin, t);
    v.push_back(t);
    cout << "Enter name of author(s):" << endl;
    //Discards \n added in previous cin
    std::getline(cin, t);
    v.push_back(t);
    cout << "Enter article text:" << endl;
    t = "";
    string temp;
    while(std::getline(cin, temp) && !temp.empty()) {
        t += temp; //Fix that i need to hit backspace twice
    }
    v.push_back(t);
}

void addReadDeleteInfo(vector<string>& v) {
    unsigned int a;
    string t;
    bool b;
    do {
        cout << "Enter the number of the article which you want to " + v[1] << endl;
        cin >> a;
        t = std::to_string(a);
        cout << "Are you sure that you want to " + v[1] +  " article " + t + " from newsgroup " + v.back() + "?(Yes = 1, No = 0)"<< endl;
        cin >> b;
    } while (!b);
    v.push_back(t);
}

void addGroupInfo(vector<string>& v) {
    string t;
    unsigned int u;
    if (v[1] == "NEW") {
        cout << "Enter the name of the group to create:" << endl;
        cin >> t;
        v.push_back(t);
    } else {
        cout << "Enter the number of the group to delete:" << endl;
        cin >> u;
        t = std::to_string(u);
        v.push_back(t);
    }
}


void inputError(string s){
    std::cout << "Invalid input!\n" + s << endl;
    exit(-1);
}

void listCommands()
{
    cout << "---Welcome to the Usenet client---" << endl;
    cout << "Following commands are possible:" << endl;
    cout << "ls - list all news groups and their numbers" << endl;
    cout << "group FLAG - execute action FLAG on newsgroup, possible flags: NEW, DEL" << endl;
    cout << "lsarticles GROUPNR - list all articles in newsgroup with number GROUPNR" << endl;
    cout << "article FLAG GROUPNR - execute action FLAG on article in group GROUPNR, possible flags: NEW, DEL, READ" << endl;
    cout << "----------------------------------" << endl;
    cout << "Input:";
}


void fillVec(string &line, char delim, vector<string> &vec)
{
	size_t start;
	size_t end = 0;

	while ((start = line.find_first_not_of(delim, end)) != string::npos) //While not at end of string, search from "end" for non-whitespace
	{
		end = line.find(delim, start); // search from start after whitespace
		vec.push_back(line.substr(start, end - start)); 
	}
}

vector<string> buildInput()
{
    string s;
    std::getline(std::cin >> std::ws, s);
    vector<string> temp{};
    fillVec(s,' ', temp);
    auto sz = temp.size();

    //Input error checking
    if(sz == 0){
        inputError("");
    }
    if(sz < 2 && temp[0] != "ls") {
        inputError("");
    }
    if(std::find(commands.begin(), commands.end(), temp[0]) == commands.end()) {
        inputError("Invalid command");
    }
    if (sz == 2) {
        if (temp[0] == "lsarticles") {
            if (temp[1].find_first_not_of("0123456789") != std::string::npos) {
                inputError("Group number must be an integer");
            }
        } else if (temp[1] != "NEW" && temp[1] !="DEL") {
            inputError("Invalid flag");
        } else {
            addGroupInfo(temp);
        }
    }
    if (sz > 2) {
        if(std::find(flags.begin(), flags.end(), temp[1]) == flags.end()) {
            inputError("Invalid flag");
        }
        if (temp[1] == "NEW") {
            addArticleInfo(temp);
        } else {
            addReadDeleteInfo(temp);
        }
    }
    return temp;
}

Connection init(int argc, char* argv[])
{
        if (argc != 3) {
                std::cerr << "Usage: myclient host-name port-number" << endl;
                exit(1);
        }

        int port = -1;
        try {
                port = std::stoi(argv[2]);
        } catch (std::exception& e) {
                std::cerr << "Wrong port number. " << e.what() << endl;
                exit(2);
        }

        Connection conn(argv[1], port);
        if (!conn.isConnected()) {
                std::cerr << "Connection attempt failed" << endl;
                exit(3);
        }

        return conn;
}


int main(int argc, char* argv[])
{
    Connection conn = init(argc, argv);
    MessageHandler m{};
    while(true) {
        listCommands();
        vector<string> vec = buildInput();
        m.msgServer(conn, vec);
        string reply = m.readString(conn);
        cout << reply << endl;
    }
    return 0;
}
