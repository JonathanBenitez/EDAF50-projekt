#include "messageHandler.h"
#include <iomanip>
#include <algorithm>
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

int readNumber(const Connection& conn)
{
        unsigned char byte1 = conn.read();
        unsigned char byte2 = conn.read();
        unsigned char byte3 = conn.read();
        unsigned char byte4 = conn.read();
        return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
}

void toBinary(std::vector<unsigned int>& buf, int i){
    buf.push_back(i>>24);
    buf.push_back(i>>16);
    buf.push_back(i>>8);
    buf.push_back(i);
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
        s += std::to_string(readNumber(conn)) + " "; //art num
        conn.read(); //throw away PAR_STRING
        int tokens = readNumber(conn);
        addTokens(conn, s, tokens);
        s += "\n";
    }
}

void displayinfo(const Connection& conn, std::string& s){
    for (int i = 0; i < 3; ++i){
        conn.read(); //remove PAR_STRING
        int j = readNumber(conn);
        for (int k = 0; k < j; ++k){
            s.push_back(conn.read());
        }
        s.push_back('\n');
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
                conn.read(); //remove ERR_NG
            }
            break;
        }
        case 22: {
            ch = conn.read();
            if(ch == 28) { //ANS_ACK
                s += "Newsgroup successfully deleted\n";
            } else { //ANS_NAK
                s += "Newsgroup could not be deleted, no group with that number exists\n";
                conn.read(); //remove ERR_NG
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
                conn.read(); //remove ERR_NG
            }
            break;
        } //art created/not created
        case 25: {
            ch = conn.read();
            if(ch == 28) { //ANS_ACK
                s += "Article successfully deleted\n";
            } else { //ANS_NAK
                s += "Article could not be deleted, no article with that number exists in the given newsgroup\n";
                conn.read(); //remove ERR_NG
            }
            break;
        } //art deleted/not deleted
        case 26: {
            ch = conn.read();
            if(ch == 28) { //ANS_ACK
                displayinfo(conn,s);
            } else { //ANS_NAK
                s += "Article could not be read, no article with that number exists in the given newsgroup\n";
                conn.read(); //remove ERR_NG
            }
            break;
        }
    }
}

std::string MessageHandler::readString(const Connection& conn)
{
        std::string s;
        char   ch;
        ch = conn.read();
        interpretAnswer(ch, conn, s);
        ch = conn.read(); //remove ANS_END
        return s;
}

