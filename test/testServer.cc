#include "connection.h"
#include "connectionclosedexception.h"
#include "server.h"
#include "command.h"
#include "databaseHandler.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using namespace std;

/*
 * Read an message from a client.
 */
vector<unsigned int> readMessage(const shared_ptr<Connection>& conn)
{
    vector<unsigned int> inpt{};
    unsigned int g{0};
    while ((g = conn->read()) != 8) {
        inpt.push_back(g);
    }
    return inpt;
}

vector<unsigned int> listNewsGroups(DatabaseHandler& db) {
    vector<unsigned int> ans{};
    vector<string> tp{};
    //tp = db.executeCommand<vector<string>>(Command_1(1));
    tp = db.executeCommand(Command_1(1));
    ans.push_back(20); //ANS_LIST_NG
    //Push back PAR_NUM, size of returned vector, for-loop that also reads size of curr vector.
    for (string st : tp){
        for(char c : st) {
            ans.push_back(c);
        }
    }
    return ans;
}

vector<unsigned int> createNG(vector<unsigned int> msg, DatabaseHandler& db) {
    string name;
    for (unsigned int i = 3; i < msg[2]+3;i++) {
        name.push_back(char(msg[i]));
    }
    //string flag = db.executeCommand<string>(Command_2(2, name));
    string flag = db.executeCommand(Command_2(2, name));
    vector<unsigned int> ans{};
    ans.push_back(21); //ANS_CREATE_NG
    if (flag == "ANS_NAK") {
        ans.push_back(29); //ANS_NACK
    } else {
        ans.push_back(28); //ANS_ACK
    }
    return ans;
    
}

void inputError() {
    throw ConnectionClosedException();
}

vector<unsigned int> interpretMsg(vector<unsigned int> msg, DatabaseHandler& db) {
    switch(msg[0]){
        case 1: return listNewsGroups(db);
        case 2: return createNG(msg,db);
//         case 3: return deleteNG(msg);
//         case 4: return listArticles(msg);
//         case 5: return createArticle(msg);´
//         case 6: return deleteArticle(msg);
//         case 7: return getArticle(msg);
        default: throw ConnectionClosedException();
    }
}


/*
 * Send a string to a client.
 */
void writeString(const shared_ptr<Connection>& conn, const vector<unsigned int>& s)
{
        for (unsigned int c : s) {
                conn->write(c);
        }
        conn->write('$');
}

Server init(int argc, char* argv[])
{
        if (argc != 2) {
                cerr << "Usage: myserver port-number" << endl;
                exit(1);
        }

        int port = -1;
        try {
                port = stoi(argv[1]);
        } catch (exception& e) {
                cerr << "Wrong format for port number. " << e.what() << endl;
                exit(2);
        }

        Server server(port);
        if (!server.isReady()) {
                cerr << "Server initialization error." << endl;
                exit(3);
        }
        return server;
}

int main(int argc, char* argv[])
{
        auto server = init(argc, argv);
        string s;
        cout << "Would you like to connect the server to a local database or disk?(L/D)\n";
        cin >> s;
        cin.ignore();
        if (s != "L" && s != "D"){
            cout << "Invalid database choice, exiting...\n";
            return -1;
        }
        auto db = DatabaseHandler(s);
        
        while (true) {
                auto conn = server.waitForActivity();
                if (conn != nullptr) {
                        try {
                                vector<unsigned int> msg = readMessage(conn);
                                vector<unsigned int> result = interpretMsg(msg,db);
                                result.push_back(27); //ANS_END
                                writeString(conn, result);
                        } catch (ConnectionClosedException&) {
                                server.deregisterConnection(conn);
                                cout << "Client closed connection" << endl;
                        }
                } else {
                        conn = make_shared<Connection>();
                        server.registerConnection(conn);
                        cout << "New client connects" << endl;
                }
        }
        return 0;
}

