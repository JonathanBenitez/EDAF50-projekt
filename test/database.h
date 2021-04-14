#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED

#include <iostream>
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
    virtual bool addArticle(string author, string artName, string article, int ngNum) = 0;
    virtual vector<string> listArticles(int newsGroupNumber) = 0;
    virtual vector<string> listNG()= 0;
    virtual string addNG(string name) = 0;
    virtual int remArticle(int articleNum, int newsGroup) = 0;
    virtual int deleteNG(string newsGroup) = 0;
    virtual vector<string> getArticle(int articleNum, int newsGroup) = 0;
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
    bool hasNGNum(int i){
        for (auto p : db) {
            if (p.first.first == i) {
                return true;
            }
        }
        return false;
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
           s.push_back(std::to_string(myPair.first.first));
           s.push_back(" " + myPair.first.second + "\n");
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
    
    int getArtNum(int ngNum) { //Gets the largest current article number
        for (const auto &myPair: db) {
            if (myPair.first.first == ngNum) {
                if (myPair.second.size() > 0) {
                    return (std::prev(myPair.second.end()))->first;
                }
                return 1;
            }
        }
        return -1;
    }

    bool addArticle(string author, string artName, string article, int ngNum){ 
        if(!hasNGNum(ngNum)){
            return false;
        }
        int articleNum = getArtNum(ngNum);
        putArt(ngNum, articleNum, artName, author, article);
            return true;
    };

    vector<string> listArticles(int newsGroupNumber){       
        vector<string> s{};
        if(!hasNGNum(newsGroupNumber)){
            return s;
        }
        for (auto& p : db) {
            if (p.first.first == newsGroupNumber) {
                for (map<int,vector<string>>::reverse_iterator iter = p.second.rbegin(); iter != p.second.rend(); ++iter){
                    s.push_back(std::to_string(iter->first));
                    s.push_back(" - Article name: " + iter->second[1] + " Author: " + iter->second[0] + "\n");
                }
                break;
            }
        }
        return s; 
    };

    int remArticle(int articleNum, int newsGroup){
        for (auto& p : db) {
            if (p.first.first == newsGroup) {
               auto it = (p.second).find(articleNum);
               (p.second).erase(it);
               return 1;
            }
        }
        return 0;
    };

    int deleteNG(string newsGroup){
        for(auto& p : db) { 
            if(p.first.first == std::stoi(newsGroup)){
               auto it = db.find(p.first);
               db.erase(it);
               return 1;
            }
        }
        return 0;
    };
    vector<string> getArticle(int articleNum, int newsGroup){ //should we do another with artName and author?
        for(auto& p : db) {
            if(p.first.first == newsGroup){
               for(auto& art: p.second){
                   if(art.first == articleNum){
                       return art.second;
                   }
               }
            }
        }
        vector<string> temp{};
        return temp;
    };

};

//TODO Implement the file-version
class FileDatabase : public Database{
    LocalDatabase db;
    string filepath;
    void parseAndFill(string& s) {
        size_t start = 4;
        size_t end;
        int ngIndex = 0;
        while ((end = s.find_first_of("@#", start+1)) != string::npos) //@ -> NG, # -> Art
        {
            if (s[start] == '@') {
                auto ng = s.substr(start+1, end-(start+1));
                std::cout << "We find NG part: " << ng << std::endl;
                std::istringstream iss(ng);
                string ngName;
                iss >> ngIndex >> ngName;
                db.putNG(ngIndex,ngName);
            } else {
                auto art = s.substr(start, end-start);
                std::cout << "We find ART part: " << art << std::endl;
                std::istringstream iss(art);
                string artInfo;
                string author;
                string title;
                int artNum;
                iss >> artNum >> author >> title>> artInfo;
                db.putArt(ngIndex,artNum,title,author,artInfo);
            }
            start = end+5; 
        }
    };
    void init(){
        ifstream infile;
        infile.open(filepath + ".txt", std::fstream::out);
        string s((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        parseAndFill(s);
        infile.close();
        std::cout << "Constructor finished\n";
    };
    void dest() {
        std::cout << "Destructor called\n";
        ofstream outfile;
        outfile.open(filepath + ".txt");
        string s;
        for (const auto &ng : db.getStructure()) {
            s.append("@@@@@");
            s.append(std::to_string(ng.first.first)); //newsgroup
            s.append(ng.first.second); 
            for (const auto &art : ng.second){
                s.append("#####");
                s.append(std::to_string(art.first)); //articleNumber
                s.append(art.second[0]);            //article Name
                s.append(art.second[1]);               //author
                s.append(art.second[2]);               //article
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
    bool addArticle(string author, string artName, string article, int ng){ 
        return db.addArticle(author, artName, article, ng);
    };
    vector<string> listArticles(int newsGroup){    
        return db.listArticles(newsGroup);
    };
    int remArticle(int articleNum, int newsGroup){
        return db.remArticle(articleNum, newsGroup);
    };
    int deleteNG(string newsGroup){
        return db.deleteNG(newsGroup);
    };
    vector<string> getArticle(int articleNum, int newsGroup){
        return db.getArticle(articleNum, newsGroup);
    };

    
};
#endif // DATABASE_H_INCLUDED
