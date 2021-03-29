#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED


#include <string>
using std::string;
#include <fstream>
using std::ofstream;
using std::ifstream;
#include <sstream>
#include <map>
#include <algorithm>
#include <vector>
using std::map;
using std::pair;
using std::vector;

class Database {
public:
    virtual ~Database(){};
    int groupcounter = 0;
   // virtual int addArticle();
   // virtual string listArticles();
    virtual vector<string> listNG()= 0;
    virtual string addNG(string name) = 0;
   // virtual int remArticle();
};

class LocalDatabase : public Database{
private:
    map<pair<int,string>,map<int,string>> db{};
    bool hasNG(string& name){
        for (auto p : db) {
            if (p.first.second == name) {
                return 1;
            }
        }
        return 0;
    };
public:
    LocalDatabase(){}
    ~LocalDatabase(){};
    map<pair<int,string>,map<int,string>>& getStructure() {
        return db;
    }
    void putNG(int i, string& s){
        db.map::emplace(std::make_pair(i,s),map<int, string>{});
    }
    void putArt(int ngNum,int artNum, string& s){
        for (auto &myPair : db) {
           if(myPair.first.first == ngNum) {
                myPair.second.emplace(artNum,s);
           }
        }
    }
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
        putNG(groupcounter,name);
        return "";
    };
};

//TODO Implement the file-version
class FileDatabase : public Database{
    LocalDatabase db;
    string filepath;
    void parseAndFill(string& s) {
        size_t start = 0;
        size_t end;
        int ngIndex = 0;
        while ((end = s.find_first_of("@#", start)) != string::npos) //@ -> NG, # -> Art
        {
            if (s[start] == '@') {
                if(s[end+4] != '@') {
                    while((end = s.find_first_of("@", end+1)) != string::npos) {
                        if (s[end+4] == '@') {
                            break;
                        }
                    }
                }
                auto ng = s.substr(start, end-start);
                std::istringstream iss(ng);
                string ngName;
                iss >> ngIndex >> ngName;
                db.putNG(ngIndex,ngName);
            } else {
                if(s[end+4] != '#') {
                    while((end = s.find_first_of("#", end+1)) != string::npos) {
                        if (s[end+4] == '#') {
                            break;
                        }
                    }
                }
                auto art = s.substr(start, end-start);
                std::istringstream iss(art);
                string artInfo;
                int artNum;
                iss >> artNum >> artInfo;
                db.putArt(ngIndex,artNum,artInfo);
            }
            start = end+5; 
        }
    };
    void init(){
        ifstream infile;
        infile.open(filepath + ".txt");
        string s((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        parseAndFill(s);
        infile.close();
    };
    void dest() {
        ofstream outfile;
        outfile.open(filepath + ".txt");
        string s;
        for (const auto &ng : db.getStructure()) {
            s.append("@@@@@");
            s.append(std::to_string(ng.first.first));
            s.append(ng.first.second);
            for (const auto &art : ng.second){
                s.append("#####");
                s.append(std::to_string(art.first));
                s.append(art.second);
                s.append("#####");
            }
            s.append("@@@@@");
        }
        std::copy(s.begin(), s.end(), std::ostreambuf_iterator<char>(outfile));
        outfile.close();
    }
  public:
    FileDatabase(string fp) {
        filepath = fp;
        db = LocalDatabase();
        init();
    }
    ~FileDatabase(){
        dest();
    }
    //Add destructor that writes to file.
    vector<string> listNG(){
        return db.listNG();
    };
    string addNG(string name) {
        return db.addNG(name);
    };
    
};
#endif // DATABASE_H_INCLUDED
