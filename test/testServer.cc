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

#define INT_SIZE 32

using namespace std;


int bitToNumber(vector<unsigned int> vec) {
    int accum = 0;
    for (int i = 0; i<INT_SIZE;++i) {
        accum += vec[i]*pow(2,i);
    }
    return accum;
}

void toBinary(std::vector<unsigned int>& buf, int i){
    for (int j = 0; j < INT_SIZE; ++j) {
        buf.push_back((i >> j) & 1);
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
        for (int i = 0; i < tp.size(); i++) {
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
        for (unsigned int i = 2; i < 2+INT_SIZE; i++) {
                bits.push_back(msg[i]);
        }
        int g = bitToNumber(bits);
        for (unsigned int i = 34; i < 34+g; i++)
        {
                name.push_back(char(msg[i]));
        }
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
        for (unsigned int i = 2; i < 2+INT_SIZE; i++) {
                bits.push_back(msg[i]);
        }
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
        for (unsigned int i = 2; i < 2+INT_SIZE; i++)
        {
                bits.push_back(msg[i]);
        }
        int ngNum  = bitToNumber(bits);
        vector<string> vec;
        vec = db.executeCommand(Command(4, std::to_string(ngNum)),vec);
        vector<unsigned int> ans{};
        ans.push_back(23); //ANS_LIST_ART
        if(vec.size() > 0){
            ans.push_back(28); //ANS_ACK
            ans.push_back(41); //PAR_NUM
            toBinary(ans, vec.size()/2);
            for(int i = 0; i < vec.size(); ++i) { 
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
        vector<unsigned int> bits{};
        bits.reserve(32);
        for (unsigned int i = 2; i < 2+INT_SIZE; i++)
        {
            bits.push_back(msg[i]);
        }
        vector<string> info{};
        int NGnum = bitToNumber(bits);
        info.push_back(std::to_string(NGnum));
        bits.clear();
        for (unsigned int i = 35; i < 35+INT_SIZE; i++) {
            bits.push_back(msg[i]);
        }
        string title;
        int titleSize = bitToNumber(bits);
        bits.clear();
        int ptr = 67;
        for (unsigned int i = ptr; i < ptr+titleSize;++i){
            title.push_back(msg[i]);
        }
        info.push_back(title);
        //cout << "Title pushed back: " << title << " title size: " << titleSize << " ,ptr value " << ptr << "\n";
        ptr += titleSize+1;
        for (unsigned int i = ptr; i < ptr+INT_SIZE; i++) {
            bits.push_back(msg[i]);
        }
        int authorSize = bitToNumber(bits);
        string author;
        bits.clear();
        ptr += INT_SIZE;
        for (unsigned int i = ptr; i<ptr+authorSize;++i) {
            author.push_back(msg[i]);
        }
        info.push_back(author);
        //cout << "Author pushed back: " << author << " author size: " << authorSize << " ,ptr value " << ptr << "\n";
        ptr += authorSize+1;
        for (unsigned int i = ptr; i < ptr+INT_SIZE; i++) {
            bits.push_back(msg[i]);
        }
        int textSize = bitToNumber(bits);
        bits.clear();
        //cout << "Textsize successfully read: " << textSize << endl;
        string text;
        ptr += INT_SIZE;
        for (unsigned int i = ptr; i<ptr+textSize;++i) {
            text.push_back(msg[i]);
        }
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
        for (unsigned int i = 2; i < 34; i++)
        {
                ngBits.push_back(msg[i]);
        }
        for (unsigned int i = 35; i < 67; i++){
                artBits.push_back(msg[i]);
        }
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
        for (unsigned int i = 2; i < 34; i++)
        {
                ngBits.push_back(msg[i]);
        }
        for (unsigned int i = 35; i < 67; i++){
                artBits.push_back(msg[i]);
        }
        int ngNum = bitToNumber(ngBits);
        int artNum = bitToNumber(artBits);
        cout << "Trying to read article " << artNum << " from newsgroup " << ngNum << endl;
        string info{std::to_string(ngNum) + " " + std::to_string(artNum)};
        vector<string> res;
        res = db.executeCommand(Command(7,info),res);
        
        for (auto s : res) {
            cout << s << endl;
        }
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
