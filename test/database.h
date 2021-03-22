#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED


#include <string>
using std::string;
#include <fstream>
using std::ofstream;
using std::ifstream;
#include <map>
#include <algorithm>
#include <vector>
using std::map;
using std::pair;
using std::vector;

class Database {
public:
   // virtual ~Database(){};
    int groupcounter = 0;
   // virtual int addArticle();
   // virtual string listArticles();
    virtual vector<string> listNG()= 0;
    virtual string addNG(string name) = 0;
   // virtual int remArticle();
};

//TODO Implement the file-version
class FileDatabase : public Database{
    string filepath;
  public:
    FileDatabase(string fp) {filepath = fp;}
    vector<string> listNG(){
        ifstream file(filepath);
        vector<string> s{};
        //Print all newsgroups 
        return s;
    };
    string addNG(string name) {
        return "";
    };
    
};

class LocalDatabase : public Database{
private:
    map<pair<int,string>,map<int,string>> db{};
    bool hasNG(string name){
        for (auto p : db) {
            if (p.first.second == name) {
                return 1;
            }
        }
        return 0;
    };
public:
    LocalDatabase(){}
    vector<string> listNG(){
        vector<string> s{};
        for (const auto &myPair : db) {
           s.push_back(std::to_string(myPair.first.first) + " " + myPair.first.second + "\n");
        }
        return s;
    };
    string addNG(string name){
        if (hasNG(name)) {
            return "ANS_NAK";
        }
        groupcounter++;
        db.map::emplace(std::make_pair(groupcounter,name),map<int, string>{});
        return "";
    };
};
#endif // DATABASE_H_INCLUDED
