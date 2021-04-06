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
    virtual void addArticle(string author, string artName, int articleNum, string article, string ng);
    virtual vector<string> listArticles(string newsGroup);
    virtual vector<string> listNG()= 0;
    virtual string addNG(string name) = 0;
    virtual int remArticle(int articleNum, string newsGroup);
    virtual int deleteNG(string newsGroup);
    virtual string getArticle(int articleNum, string newsGroup);
};

class LocalDatabase : public Database{
private:
    map<pair<int,string>,map<int,vector<string>>> db{}; // map< pair<GNum,GName>,map<artNum,[artName, Author, article]> >
    bool hasNG(string& name){
        for (auto p : db) {
            if (p.first.second == name) {
                return 1;
            }
        }
        return 0;
    };
    int getNGNum(string name){
        for (auto p : db) {
            if (p.first.second == name) {
                return p.first.first;
            }
        }
        return 0; //TODO: send error?
    };
public:
    LocalDatabase(){}
    ~LocalDatabase(){};
    map<pair<int,string>,map<int,vector<string>>>& getStructure() {
        return db;
    }

    void putNG(int i, string& s){
        db.map::emplace(std::make_pair(i,s),map<int, vector<string>>{});
    }

    void putArt(int ngNum,int artNum, string articleName, string author, string& article){
        auto artVec = {articleName,author, article};
        for (auto &myPair : db) {
           if(myPair.first.first == ngNum) {
                myPair.second.emplace(artNum, artVec);
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

    void addArticle(string author, string artName, int articleNum, string& article, string newsGroup){ 
        if(!hasNG(newsGroup)){
            addNG(newsGroup);
            //TODO: Send error or add new news group? 
        }
        int ngNum = getNGNum(newsGroup);
        putArt(ngNum, articleNum, artName, author, article);
      
         //return art num or group num or nothing? 
    };

    vector<string> listArticles(string newsGroup){       
        vector<string> s{};
        if(!hasNG(newsGroup)){
            //TODO: Send error?
            return s;
        }
        for (auto p : db) {
              
            if (p.first.second == newsGroup) {
                for (map<int,vector<string>>::reverse_iterator iter = p.second.rbegin(); iter != p.second.rend(); ++iter){
                    s.push_back("Article name: " + iter->second[0] + " Author: " + iter->second[1] + "\n");
                }
                return;
            }
        }
        return s; 
    };

    int remArticle(int articleNum, string newsGroup){
         if(!hasNG(newsGroup)){
            //TODO: Send error?
            return 0;
        }
        for (auto p : db) {
            if (p.first.second == newsGroup) {
               auto it = p.second.find(articleNum);
               p.second.erase(it);
               return 1;
            }
        }
        return 0;
    };

    int deleteNG(string newsGroup){
         if(!hasNG(newsGroup)){
            //TODO: Send error?
            return 0;
        }
        for(auto p : db) { 
            if(p.first.second ==  newsGroup){
               auto it = db.find(p.first);
               db.erase(it);
               return 1;
            }
        }
    };
    string getArticle(int articleNum, string newsGroup){ //should we do another with artName and author?
        for(auto p : db) { 
            if(p.first.second ==  newsGroup){
               for(auto art: p.second){
                   if(art.first == articleNum){
                       return art.second[2];
                   }
               }
            }
        }
        return newsGroup;
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
    int addArticle(string author, int articleNum, string article, string newsGroup){
        return 0;
    };
    vector<string> listArticles(string newsGroup){
        vector<string> s{};
        return s;
    };
    int remArticle(int articleNum, string newsGroup){
        return 0;
    };
    int deleteNG(string newsGroup){
        return 0;
    };
    string getArticle(int articleNum, string newsGroup){
        return newsGroup;
    };

    
};
#endif // DATABASE_H_INCLUDED
