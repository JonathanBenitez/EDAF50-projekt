#include "messageHandler.h"
#include <iomanip>
#include <utility>
#include <vector>
#include <string>
#include <string.h>
#include "protocol.h"
#include <iostream>
#include <math.h>
MessageHandler::MessageHandler()
{
}


void pushString(const std::string& v, std::vector<unsigned int>& buffer){
    const char* charBuffer = v.c_str();
    for (size_t i = 0; i < v.size(); i++) {
        buffer.push_back(charBuffer[i]);
    }
}

void toBinary(std::vector<unsigned int>& buf, int i){
    for (int j = 0; j < 32; ++j) {
        buf.push_back((i >> j) & 1);
    }
}
int bitToNumber(std::vector<unsigned int> vec) {
    int accum = 0;
    for (int i = 0; i<32;++i) {
        accum += vec[i]*pow(2,i);
    }
    return accum;
}

void MessageHandler::match(std::vector<std::string>& v, std::vector<unsigned int>& buffer) {
    if (v[0] == "group") {
        buffer.push_back(int(stringToProtcolTable[v[1]]));
        if (v[1] == "NEW") {
             buffer.push_back(int(stringToProtcolTable["string"]));
             toBinary(buffer, v[2].size());
             pushString(v[2],buffer);
        } else {
            buffer.push_back(int(stringToProtcolTable["number"]));
            toBinary(buffer, std::stoi(v[2]));
        }
    } else { // v[0] == article
        if (v[1] == "NEW") {
             buffer.push_back(int(stringToProtcolTable["newart"]));
             buffer.push_back(int(stringToProtcolTable["number"]));
             toBinary(buffer, std::stoi(v[2]));
             buffer.push_back(int(stringToProtcolTable["string"]));
             toBinary(buffer, v[3].size());
             pushString(v[3],buffer);
             buffer.push_back(int(stringToProtcolTable["string"]));
             toBinary(buffer, v[4].size());
             pushString(v[4],buffer);
             buffer.push_back(int(stringToProtcolTable["string"]));
             toBinary(buffer, v[5].size());
             pushString(v[5],buffer);
        } else { //v[1] == "DEL" || "READ"
            if (v[1] == "DEL") {
                buffer.push_back(int(stringToProtcolTable["delart"]));
            } else {
                buffer.push_back(int(stringToProtcolTable["READ"]));
            }
            buffer.push_back(int(stringToProtcolTable["number"])); //NGnumber
            toBinary(buffer,std::stoi(v[2]));
            buffer.push_back(int(stringToProtcolTable["number"])); //Artnumber
            toBinary(buffer,std::stoi(v[3]));
            }
    }
}


void MessageHandler::msgServer(Connection& c, std::vector<std::string>& v)
{
    std::vector<unsigned int> buffer{};
    switch (v.size()){
        case 1: buffer.push_back(int(stringToProtcolTable[v[0]]));
                break;
        case 2: buffer.push_back(int(stringToProtcolTable[v[0]]));
                buffer.push_back(int(stringToProtcolTable["number"]));
                toBinary(buffer, std::stoi(v[1]));
                break;
        default: match(v,buffer);
    };
    buffer.push_back(int(stringToProtcolTable["END"]));
    for (unsigned int i : buffer) {
        c.write(i);
    }
}

void fillText(std::string& s, const Connection& conn, unsigned int protocol){
    char ch;
    while ((ch = conn.read()) != protocol) {
        s += ch;
    }
}

int readNumber(const Connection& conn) {
    std::vector<unsigned int> vec{};
    for (int i = 0; i < 32; ++i) {
        vec.push_back(conn.read());
    }
    return bitToNumber(vec);
}

void addTokens(const Connection& conn, std::string& s, int tokens) {
    for (int i = 0; i < tokens; ++i) {
        s += conn.read();
    }
}

void listvals(const Connection& conn, std::string& s){
    conn.read(); //throw away PAR_NUM
    int numNGs = readNumber(conn);
    for (int i = 0; i < numNGs; ++i) {
        conn.read(); //throw away PAR_NUM
        s += std::to_string(readNumber(conn)); //art num
        conn.read(); //throw away PAR_STRING
        int tokens = readNumber(conn);
        addTokens(conn, s, tokens);
    }
}

void interpretAnswer(char ch,const Connection& conn, std::string& s) {
    
    switch(ch) {
        case 20: {
            listvals(conn,s);
            break;
        }
        case 21: {
            ch = conn.read();
            if(ch == 28) { //ANS_ACK
                s += "Newsgroup successfully created\n";
            } else { //ANS_NAK
                s += "Newsgroup could not be created, a group with that name already exists\n";
                conn.read(); //remove ERR_NG Not completely right.
            }
            break;
        }
        case 22: {
            ch = conn.read();
            if(ch == 28) { //ANS_ACK
                s += "Newsgroup successfully deleted\n";
            } else { //ANS_NAK
                s += "Newsgroup could not be deleted, no group with that number exists\n";
                conn.read(); //remove ERR_NG Not completely right.
            }
            break;
        }
        case 23: {
            ch = conn.read();
            if (ch == 28) {
                listvals(conn, s);
            } else {
                s += "No newsgroup with that number exists or no articles to list\n";
                conn.read(); //remove ERR_NG Not completely right.
            }
            break;
        } 
        case 24: {
            ch = conn.read();
            if(ch == 28) { //ANS_ACK
                s += "Article successfully created\n";
            } else { //ANS_NAK
                s += "Article could not be created, article already exists or newsgroup with the given number does not\n";
                conn.read(); //remove ERR_NG Not completely right.
            }
            break;
        } //art created/not created
        case 25: {
            ch = conn.read();
            if(ch == 28) { //ANS_ACK
                s += "Article successfully deleted\n";
            } else { //ANS_NAK
                s += "Article could not be deleted, no article with that number exists in the given newsgroup\n";
                conn.read(); //remove ERR_NG Not completely right.
            }
            break;
        } //art deleted/not deleted
        case 26: {} //read art
    }
}
// ANS_LIST_NG    = 20, // answer list newsgroups
// ANS_CREATE_NG  = 21, // answer create newsgroup
// ANS_DELETE_NG  = 22, // answer delete newsgroup
// ANS_LIST_ART   = 23, // answer list articles
// ANS_CREATE_ART = 24, // answer create article
// ANS_DELETE_ART = 25, // answer delete article
// ANS_GET_ART    = 26, // answer get article
// ANS_END        = 27, // answer end
// ANS_ACK        = 28, // acknowledge
// ANS_NAK        = 29, // negative acknowledge

std::string MessageHandler::readString(const Connection& conn)
{
        std::string s;
        char   ch;
        ch = conn.read();
        interpretAnswer(ch, conn, s);
        ch = conn.read(); //remove ANS_END
        return s;
}

