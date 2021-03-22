#ifndef DATABASEHANDLER_H_INCLUDED
#define DATABASEHANDLER_H_INCLUDED

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include "database.h"

using std::pair;
using std::map;
using std::vector;
using std::string;

class DatabaseHandler {
public:
    DatabaseHandler(string f);
    ~DatabaseHandler(){free(database);};
    
    template <typename T>
    T executeCommand(int i, string s); //Dummy empty rType since C++ is too dummy to do return value type inference 
private:
    Database* database;
};

#endif // DATABASEHANDLER_H_INCLUDED
