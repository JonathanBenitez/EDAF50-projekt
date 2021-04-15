#include "databaseHandler.h"
#include <iostream>
#include <fstream>
#include <string>
using std::string;

DatabaseHandler::DatabaseHandler(std::string f)
{
    if (f == "D") {
        string s;
        std::cout << "Please enter the disk database name" << std::endl;
        std::cin >> s;
        database = new FileDatabase(s);
        std::cout << "Initialized disk database\n";
    } else {
        database = new LocalDatabase();
        std::cout << "Initialized local database\n";
    }
}

bool DatabaseHandler::executeCommand(Command c, bool b)
{
        bool out = false;
        string msg = c.getInfo();
        switch(c.getInt()){
            case 3: {
                out = database->deleteNG(msg);
                break;
            }
            case 5: {
                std::istringstream iss(msg);
                int artNum;
                iss >> artNum;
                string author, artname, text;
                getline(iss, artname, '{');
                getline(iss, author, '{');
                getline(iss, text, '{');
                out = database->addArticle(author, artname, text, artNum);
            }
            break;
            case 6: {
                std::istringstream iss(msg);
                int ngNum,artNum;
                iss >> ngNum >> artNum;
                out = database->remArticle(artNum,ngNum);
            }
            break;
            default: return out;
        }
    return out;
}

string DatabaseHandler::executeCommand(Command c, string s)
{
    return database->addNG(c.getInfo());
}

vector<string> DatabaseHandler::executeCommand(Command c, vector<string> vec)
{
            vector<string> out;
            string msg = c.getInfo();
            switch(c.getInt()){
            case 1: out = database->listNG();
            break;
            case 4: out = database->listArticles(std::stoi(msg));
            break;
            case 7:{
                std::istringstream iss(msg);
                int ngNum,artNum;
                iss >> ngNum >> artNum;
                out = database->getArticle(artNum,ngNum);
                break;
            }
            default:return out;
            }
    return out;
}
