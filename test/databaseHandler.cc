#include "databaseHandler.h"
#include "database.h"
#include "command.h"
#include <iostream>
#include <string>
using std::string;

DatabaseHandler::DatabaseHandler(std::string f)
{
    if (f == "D") {
        string s;
        std::cout << "Please enter the database filepath" << std::endl;
        std::cin >> s;
        database = new FileDatabase(s);
    } else {
        database = new LocalDatabase();
    }
}

/*
template <typename T>
T DatabaseHandler::executeCommand(int i, std::string s)
{
        T out;
        switch(i){
        case 1: out = database->listNG();
        break;
        case 2: out = database->addNG(s);
        break;
//         case 3: return deleteNG(msg);
//         case 4: return listArticles(msg);
//         case 5: return createArticle(msg);
//         case 6: return deleteArticle(msg);
//         case 7: return getArticle(msg);
        default: return "Error! Something went wrong!";
    }
    return out;
}
*/
std::vector<std::string> DatabaseHandler::executeCommand(Command_1 c){
 std::vector<std::string> out;
        switch(c.getInt()){
        case 1: out = database->listNG();
        break;
        //case 2: out = database->addNG(c.getString());
        break;
//         case 3: return deleteNG(msg);
//         case 4: return listArticles(msg);
//         case 5: return createArticle(msg);
//         case 6: return deleteArticle(msg);
//         case 7: return getArticle(msg);
       // default: return "Error! Something went wrong!";
    }
    return out;

}

//template<> 
std::string DatabaseHandler::executeCommand(Command_2 c){
        std::string out;
        switch(c.getInt()){
        //case 1: out = database->listNG();
        break;
        case 2: out = database->addNG(c.getString());
        break;
//         case 3: return deleteNG(msg);
//         case 4: return listArticles(msg);
//         case 5: return createArticle(msg);
//         case 6: return deleteArticle(msg);
//         case 7: return getArticle(msg);
        default: return "Error! Something went wrong!";
    }
    return out;

}

