#include "messageHandler.h"
#include <iomanip>
#include <utility>
#include <vector>
#include <string>
#include <string.h>
#include "protocol.h"
#include <iostream>
MessageHandler::MessageHandler()
{
}


void pushString(const std::string& v, std::vector<unsigned int>& buffer){
    const char* charBuffer = v.c_str();
    for (size_t i = 0; i < v.size(); i++) {
        buffer.push_back(charBuffer[i]);
    }
}

void MessageHandler::match(std::vector<std::string>& v, std::vector<unsigned int>& buffer) {
    if (v[0] == "group") {
        buffer.push_back(int(stringToProtcolTable[v[1]]));
        if (v[1] == "NEW") {
             buffer.push_back(int(stringToProtcolTable["string"]));
             buffer.push_back(v[2].size());
             pushString(v[2],buffer);
        } else {
            buffer.push_back(int(stringToProtcolTable["number"]));
            buffer.push_back(std::stoi(v[2]));
        }
    } else { // v[0] == article
        if (v[1] == "NEW") {
             buffer.push_back(int(stringToProtcolTable["newart"]));
             buffer.push_back(int(stringToProtcolTable["number"]));
             buffer.push_back(std::stoi(v[3]));
             buffer.push_back(int(stringToProtcolTable["string"]));
             buffer.push_back(v[2].size());
             pushString(v[2],buffer);
             buffer.push_back(int(stringToProtcolTable["string"]));
             buffer.push_back(v[4].size());
             pushString(v[4],buffer);
             buffer.push_back(int(stringToProtcolTable["string"]));
             buffer.push_back(v[5].size());
             pushString(v[5],buffer);
        } else { //v[1] == "DEL" || "READ"
            if (v[1] == "DEL") {
                buffer.push_back(int(stringToProtcolTable["delart"]));
            } else {
                buffer.push_back(int(stringToProtcolTable["READ"]));
            }
            buffer.push_back(int(stringToProtcolTable["number"]));
            buffer.push_back(std::stoi(v[2]));
            buffer.push_back(int(stringToProtcolTable["number"]));
            buffer.push_back(std::stoi(v[3]));
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
                buffer.push_back(std::stoi(v[1]));
                break;
        default: match(v,buffer);
    };
    buffer.push_back(int(stringToProtcolTable["END"]));
    for (unsigned int i : buffer) {
        c.write(i);
    }
}

std::string MessageHandler::readString(const Connection& conn)
{
        std::string s;
        char   ch;
        while ((ch = conn.read()) != '$') {
                s += ch;
        }
        return s;
}

