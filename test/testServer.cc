#include "connection.h"
#include "connectionclosedexception.h"
#include "server.h"
#include "databaseHandler.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using namespace std;

/*
 * Read a message from a client.
 */
vector<unsigned int> readMessage(const shared_ptr<Connection> &conn)
{
        vector<unsigned int> inpt{};
        unsigned int g{0};
        while ((g = conn->read()) != 8)
        {
                inpt.push_back(g);
        }
        return inpt;
}
// 1. List newsgroups.
// The reply contains the number of newsgroups followed by the identifica- tion numbers and titles of the groups.
//    COM_LIST_NG COM_END
//   ANS_LIST_NG num_p [num_p string_p]* ANS_END

vector<unsigned int> listNewsGroups(DatabaseHandler &db)
{
        vector<unsigned int> ans{};
        vector<string> tp{};
        tp = db.executeCommand<vector<string>>(1, "");
        ans.push_back(20); //ANS_LIST_NG
        //Push back PAR_NUM, size of returned vector, for-loop that also reads size of curr vector.
        for (string st : tp)
        {
                for (char c : st)
                {
                        ans.push_back(c);
                }
        }
        
        return ans;
}

//    COM_CREATE_NG string_p COM_END
//    ANS_CREATE_NG [ANS_ACK | ANS_NAK ERR_NG_ALREADY_EXISTS] ANS_END

vector<unsigned int> createNG(vector<unsigned int> msg, DatabaseHandler &db)
{
        string name;
        for (unsigned int i = 3; i < msg[2] + 3; i++)
        {
                name.push_back(char(msg[i]));
        }
        string flag = db.executeCommand<string>(2, name);
        vector<unsigned int> ans{};
        ans.push_back(21); //ANS_CREATE_NG
        if (flag == "ANS_NAK")
        {
                ans.push_back(29); //ANS_NACK
        }
        else
        {
                ans.push_back(28); //ANS_ACK
        }
        return ans;
} //do we not have to append the error and ans end

// COM_DELETE_NG num_p COM_END
// ANS_DELETE_NG [ANS_ACK | ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
vector<unsigned int> deleteNG(vector<unsigned int> msg)
{
        vector<char> bits{};
        bits.push_back("PAR_NUM");
        for (unsigned int i = 1; i < 33; i++)
        {
                bits.push_back(char(msg[i]));
        }
        string num;                                       //some function to make it into a string. executecommand: stay with string input for consistency
        bool check = db.executeCommand<bool>(3, num);   //assuming boolean from method deleteNG, might want something else here
        vector<unsigned int> ans{};
        ans.push_back(22); //ANS_DELETE_NG
        if (check)
        {                          
                ans.push_back(28); //ANS_ACK
        }
        else
        {
                ans.push_back(29); //ANS_NAK
                ans.push_back(51); //ERR_NG_DOES_NOT_EXIST
        }
        
        return ans;
}
//finalize when return is known!!
//COM_LIST_ART num_p COM_END
//ANS_LIST_ART [ANS_ACK num_p [num_p string_p]* |   ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
vector<unsigned int> listArticles(vector<unsigned int> msg)
{
        vector<char> bits{};
        for (unsigned int i = 1; i < 33; i++)
        {
                bits.push_back(char(msg[i]));
        }
        string num; // = numToBits(bits);          
        // executeCommand to: get the list|list doesnt exist
        vector<unsigned int> ans{};
        if(the list does exist){
        ans.push_back(23); //ANS_LIST_ART
        ans.push_back(28); //ANS_ACK
        for(the list)
        ans.push_back(the numbers and strings)
        } else {
        ans.push_back(29) //ANS_NAK
        ans.push_back(51) //ERR_NG_DOES_NOT_EXIST
        
        }
        return ans;
}

//COM_CREATE_ART num_p string_p string_p string_p COM_END
//ANS_CREATE_ART [ANS_ACK | ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
vector<unsigned int> createArticles(vector<unsigned int> msg)
{
        vector<char> bits{};
        for (unsigned int i = 1; i < 33; i++)
        {
                bits.push_back(char(msg[i]));
        }
        string num; // = numToBits(bits); 
        vector<unsigned int> ans{};
        // executeCommand to create 
        ans.push_back(24);
        if(the NG exists) {
        ans.push_back(28);      //ANS_ACK
        }
        else {
        ans.push_back(29);      //ANS_NACK
        ans.push_back(51);      //ERR_NG_DOES_NOT_EXIST
        }
        
        return ans;
}

//COM_DELETE_ART num_p num_p COM_END
//ANS_DELETE_ART [ANS_ACK | ANS_NAK [ERR_NG_DOES_NOT_EXIST | ERR_ART_DOES_NOT_EXIST]] ANS_END
vector<unsigned int> deleteArticle(vector<unsigned int> msg)
{
        vector<char> ngBits{};
        vector<char> artBits{};
        for (unsigned int i = 1; i < 33; i++)
        {
                ngbits.push_back(char(msg[i]));
        }
        for (unsigned int i = 34; i < 55; i++){
                artBits.push_back(char(msg[i]));
        }
        //executeCommand to delete + check art && NG
        vector<unsigned int> ans{};
        ans.push_back(25); //ANS_DELETE_ART
        if(no NG) {
        ans.push_back(29);      //ANS_NAK
        ans.push_back(51);      //ERR_NG_DOESNT_EXIST
        } else if (no Art) {
        ans.push_back(29);      //ANS_NAK
        ans.push_back(52);       //ERR_ART_DOES_NOT_EXIST
        } else {
        ans.push_back(28);      //ANS_ACK
        }
        
        return ans;
}

//COM_GET_ART num_p num_p COM_END
//ANS_GET_ART [ANS_ACK string_p string_p string_p | ANS_NAK [ERR_NG_DOES_NOT_EXIST | ERR_ART_DOES_NOT_EXIST]] ANS_END
vector<unsigned int> getArticle(vector<unsigned int> msg)
{       
        vector<char> ngBits{};
        vector<char> artBits{};
        for (unsigned int i = 1; i < 33; i++)
        {
                ngbits.push_back(char(msg[i]));
        }
        for (unsigned int i = 34; i < 55; i++){
                artBits.push_back(char(msg[i]));
        }
        //executecommand to get article
        vector<unsigned int> ans{};             //perhaps send entire vector as string
        ans.push_back(26); //ANS_GET_ART
        if(no NG) {
        ans.push_back(29);      //ANS_NAK
        ans.push_back(51);      //ERR_NG_DOESNT_EXIST
        } else if (no Art) {
        ans.push_back(29);      //ANS_NAK
        ans.push_back(52);       //ERR_ART_DOES_NOT_EXIST
        } else {
        ans.push_back(28);      //ANS_ACK
        //push back title, author and text
        }
       
        return ans;
}

void inputError()
{
        throw ConnectionClosedException();
}

vector<unsigned int> interpretMsg(vector<unsigned int> msg, DatabaseHandler &db)
{
        switch (msg[0])
        {
        case 1:
                return listNewsGroups(db);
        case 2:
                return createNG(msg, db);
                //         case 3: return deleteNG(msg);
                //         case 4: return listArticles(msg);
                //         case 5: return createArticle(msg);´
                //         case 6: return deleteArticle(msg);
                //         case 7: return getArticle(msg);
        default:
                throw ConnectionClosedException();
        }
}

/*
 * Send a string to a client.
 */
void writeString(const shared_ptr<Connection> &conn, const vector<unsigned int> &s)
{
        for (unsigned int c : s)
        {
                conn->write(c);
        }
        conn->write('$');
}

Server init(int argc, char *argv[])
{
        if (argc != 2)
        {
                cerr << "Usage: myserver port-number" << endl;
                exit(1);
        }

        int port = -1;
        try
        {
                port = stoi(argv[1]);
        }
        catch (exception &e)
        {
                cerr << "Wrong format for port number. " << e.what() << endl;
                exit(2);
        }

        Server server(port);
        if (!server.isReady())
        {
                cerr << "Server initialization error." << endl;
                exit(3);
        }
        return server;
}

int main(int argc, char *argv[])
{
        auto server = init(argc, argv);
        string s;
        cout << "Would you like to connect the server to a local database or disk?(L/D)\n";
        cin >> s;
        cin.ignore();
        if (s != "L" && s != "D")
        {
                cout << "Invalid database choice, exiting...\n";
                return -1;
        }
        auto db = DatabaseHandler(s);

        while (true)
        {
                auto conn = server.waitForActivity();
                if (conn != nullptr)
                {
                        try
                        {
                                vector<unsigned int> msg = readMessage(conn);
                                vector<unsigned int> result = interpretMsg(msg, db);
                                result.push_back(27); //ANS_END
                                writeString(conn, result);
                        }
                        catch (ConnectionClosedException &)
                        {
                                server.deregisterConnection(conn);
                                cout << "Client closed connection" << endl;
                        }
                }
                else
                {
                        conn = make_shared<Connection>();
                        server.registerConnection(conn);
                        cout << "New client connects" << endl;
                }
        }
        return 0;
}
