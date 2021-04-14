#include "connection.h"
#include "connectionclosedexception.h"
#include "server.h"
#include "databaseHandler.h"
#include "command.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <math.h>
#include <fstream>

#define INT_SIZE 4

using namespace std;

int bitToNumber(vector<unsigned int>& vec)
{
    unsigned char byte1 = vec[0];
    unsigned char byte2 = vec[1];
    unsigned char byte3 = vec[2];
    unsigned char byte4 = vec[3];
    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
}

void toBinary(std::vector<unsigned int>& buf, int i){
    buf.push_back(i>>24);
    buf.push_back(i>>16);
    buf.push_back(i>>8);
    buf.push_back(i);
}


void pushToVector(vector<unsigned int>& msg, vector<unsigned int>& target, int& pointer, int size){
    auto end = pointer+size;
    for (; pointer < end;++pointer){
        target.push_back(msg[pointer]);
    }
}
void pushToVector(vector<unsigned int>& msg, string& target, int& pointer, int size){
    auto end = pointer+size;
    for (; pointer < end;++pointer){
        target.push_back(char(msg[pointer]));
    }
}

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
        tp = db.executeCommand(Command(1,""),tp);
        ans.push_back(20); //ANS_LIST_NG
        ans.push_back(41); //PAR_NUM
        toBinary(ans,tp.size()/2); //Push back number of groups
        for (long unsigned int i = 0; i < tp.size(); i++) {
            if(i%2 == 0) {
                ans.push_back(41); //PAR_NUM
                toBinary(ans, stoi(tp[i]));
                continue;
            } else {
                ans.push_back(40); //PAR_STRING
                toBinary(ans, tp[i].length()); //String length
                for (char c : tp[i]) {
                    ans.push_back(c);
                }
            }
        }
    return ans;
}

//    COM_CREATE_NG string_p COM_END
//    ANS_CREATE_NG [ANS_ACK | ANS_NAK ERR_NG_ALREADY_EXISTS] ANS_END

vector<unsigned int> createNG(vector<unsigned int> msg, DatabaseHandler &db)
{
        string name;
        vector<unsigned int> bits{};
        int pointer = 2;
        pushToVector(msg,bits,pointer,INT_SIZE);
        int g = bitToNumber(bits);
        pushToVector(msg,name,pointer,g);
        vector<unsigned int> ans{};
        string flag = db.executeCommand(Command(2, name),name);
        ans.push_back(21); //ANS_CREATE_NG
        if (flag == "ANS_NAK") {
                ans.push_back(29); //ANS_NACK
                ans.push_back(50); //ERR_NG_ALREADY_EXISTS
        } else {
                ans.push_back(28); //ANS_ACK
        }
        return ans;
}

// COM_DELETE_NG num_p COM_END
// ANS_DELETE_NG [ANS_ACK | ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
vector<unsigned int> deleteNG(vector<unsigned int> msg, DatabaseHandler &db)
{
        vector<unsigned int> bits{};
        int pointer = 2;
        pushToVector(msg,bits,pointer,INT_SIZE);
        int num = bitToNumber(bits);
        bool b = false;
        bool check = db.executeCommand(Command(3, std::to_string(num)),b);   //assuming boolean from method deleteNG, might want something else here
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
vector<unsigned int> listArticles(vector<unsigned int> msg, DatabaseHandler &db)
{
        vector<unsigned int> bits{};
        int pointer = 2;
        pushToVector(msg,bits,pointer,INT_SIZE);
        int ngNum  = bitToNumber(bits);
        vector<string> vec;
        vec = db.executeCommand(Command(4, std::to_string(ngNum)),vec);
        vector<unsigned int> ans{};
        ans.push_back(23); //ANS_LIST_ART
        if(vec.size() > 0){
            ans.push_back(28); //ANS_ACK
            ans.push_back(41); //PAR_NUM
            toBinary(ans, vec.size()/2);
            for(long unsigned int i = 0; i < vec.size(); ++i) { 
                if(i%2 == 0) {
                ans.push_back(41); //PAR_NUM
                toBinary(ans, stoi(vec[i]));
                } else {
                    ans.push_back(40); //PAR_STRING
                    toBinary(ans, vec[i].length()); //String length
                    for (char c : vec[i]) {
                        ans.push_back(c);
                    }
                }
            }
        } else {
            ans.push_back(29); //ANS_NAK
            ans.push_back(51); //ERR_NG_DOES_NOT_EXIST
        
        }
        return ans;
}

//COM_CREATE_ART num_p string_p string_p string_p COM_END
//ANS_CREATE_ART [ANS_ACK | ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
vector<unsigned int> createArticle(vector<unsigned int> msg, DatabaseHandler &db)
{
        //cout << "We start reading article" << endl;
        vector<unsigned int> bits{};
        bits.reserve(4);
        int pointer = 2;
        pushToVector(msg,bits,pointer,INT_SIZE);
        vector<string> info{};
        int NGnum = bitToNumber(bits);
        info.push_back(std::to_string(NGnum));
        //cout << "Pushed back NGnum successfully: " << NGnum;
        bits.clear();
        ++pointer;
        pushToVector(msg,bits,pointer,INT_SIZE);
        string title;
        int titleSize = bitToNumber(bits);
        bits.clear();
        pushToVector(msg,title,pointer,titleSize);
        info.push_back(title);
        //cout << "Title pushed back: " << title << " title size: " << titleSize << " ,ptr value " << pointer << "\n";
        ++pointer;
        pushToVector(msg,bits,pointer,INT_SIZE);
        int authorSize = bitToNumber(bits);
        string author;
        bits.clear();
        pushToVector(msg,author,pointer,authorSize);
        info.push_back(author);
        //cout << "Author pushed back: " << author << " author size: " << authorSize << " ,ptr value " << pointer << "\n";
        ++pointer;
        pushToVector(msg,bits,pointer,INT_SIZE);
        int textSize = bitToNumber(bits);
        bits.clear();
        //cout << "Textsize successfully read: " << textSize << endl;
        string text;
        pushToVector(msg,text,pointer,textSize);
        info.push_back(text);
        string fullstring;
        for (auto s : info) {
            fullstring += s;
            fullstring += " ";
        }
        //cout << "ARTICLE INFO: " + fullstring;
        bool b = false;
        bool a = db.executeCommand(Command(5,fullstring), b); //Info contains [ngnum, artnum, author, text]
        vector<unsigned int> ans{};
        ans.push_back(24);
        if(a) {
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
vector<unsigned int> deleteArticle(vector<unsigned int> msg, DatabaseHandler &db)
{
        vector<unsigned int> ngBits{};
        vector<unsigned int> artBits{};
        int pointer = 2;
        pushToVector(msg,ngBits,pointer,INT_SIZE);
        ++pointer;
        pushToVector(msg,artBits,pointer,INT_SIZE);
        int ngNum = bitToNumber(ngBits);
        int artNum = bitToNumber(artBits);
        string info{std::to_string(ngNum) + " " +  std::to_string(artNum)};
        bool b = false;
        bool answer = db.executeCommand(Command(6, info), b);
        vector<unsigned int> ans{};
        ans.push_back(25); //ANS_DELETE_ART
        if(!answer) {
            ans.push_back(29);      //ANS_NAK
            ans.push_back(51);      //ERR_NG_DOESNT_EXIST
        } else {
            ans.push_back(28);      //ANS_ACK
        }
        return ans;
}

//COM_GET_ART num_p num_p COM_END
//ANS_GET_ART [ANS_ACK string_p string_p string_p | ANS_NAK [ERR_NG_DOES_NOT_EXIST | ERR_ART_DOES_NOT_EXIST]] ANS_END
vector<unsigned int> getArticle(vector<unsigned int> msg, DatabaseHandler &db)
{       
        vector<unsigned int> ngBits{};
        vector<unsigned int> artBits{};
        int pointer = 2;
        pushToVector(msg,ngBits,pointer,INT_SIZE);
        ++pointer;
        pushToVector(msg,artBits,pointer,INT_SIZE);
        int ngNum = bitToNumber(ngBits);
        int artNum = bitToNumber(artBits);
        //cout << "Trying to read article " << artNum << " from newsgroup " << ngNum << endl;
        string info{std::to_string(ngNum) + " " + std::to_string(artNum)};
        vector<string> res;
        res = db.executeCommand(Command(7,info),res);
        vector<unsigned int> ans{};             //perhaps send entire vector as string
        ans.push_back(26); //ANS_GET_ART
        if(res.size() == 0) {
            ans.push_back(29);      //ANS_NAK
            ans.push_back(51);      //ERR_NG_DOESNT_EXIST
        } else {
            ans.push_back(28);      //ANS_ACK
            for (auto info : res) {
                ans.push_back(40); //PAR_STRING
                toBinary(ans,info.length());
                for (auto ch : info) {
                    ans.push_back(ch);
                }
            }
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
                return createNG(msg,db);
        case 3: 
                return deleteNG(msg,db);
        case 4: 
                return listArticles(msg,db);
        case 5: 
                return createArticle(msg,db);
        case 6: 
                return deleteArticle(msg,db);
        case 7: 
                return getArticle(msg,db);
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
