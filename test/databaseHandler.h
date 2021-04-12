#ifndef DATABASEHANDLER_H_INCLUDED
#define DATABASEHANDLER_H_INCLUDED

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include "database.h"
#include "command.h"

using std::pair;
using std::map;
using std::vector;
using std::string;

class DatabaseHandler {
public:
    DatabaseHandler(string f);
    ~DatabaseHandler(){free(database);};
    bool executeCommand(Command c,bool b);
    vector<string> executeCommand(Command c,vector<string> vec);
    string executeCommand(Command c,string s);
    
//     template <typename T>
//     T executeCommand(Command c) {
//             T out;
//             auto msg = c.getInfo();
//             
//             switch(c.getInt()){
//             case 1: out = database->listNG();
//             break;
//             case 2: out = database->addNG(msg);
//             break;
//             case 3: out = database->deleteNG(msg);
//             break;
//             case 4: out = database->listArticles(std::stoi(msg));
//             break;
//             case 5: {
//                 std::istringstream iss(msg);
//                 int artNum;
//                 string author, artname, text;
//                 iss >> artNum >> author >> artname >> text;
//                 out = database->addArticle(author, artname, text, artNum);
//             }
//             break;
//             case 6: {
//                 std::istringstream iss(msg);
//                 int ngNum,artNum;
//                 iss >> ngNum >> artNum;
//                 out = database->remArticle(artNum,ngNum);
//             }
//             break;
//             case 7:{
//                 std::istringstream iss(msg);
//                 int ngNum,artNum;
//                 iss >> ngNum >> artNum;
//                 out = database->remArticle(artNum,ngNum);
//             }
//             break;
//             default: return out;
//         }
//         return out;
//     }
private:
    Database* database;
};

#endif // DATABASEHANDLER_H_INCLUDED
