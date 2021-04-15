#ifndef MESSAGEHANDLER_H_INCLUDED
#define MESSAGEHANDLER_H_INCLUDED

#include "connection.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "protocol.h"

class MessageHandler {
public:
    MessageHandler();
    void msgServer(Connection& c, std::vector<std::string>& v);
    void match(std::vector<std::string>& v, std::vector<unsigned int>& buffer);
    std::string readString(const Connection& c);
private:
    std::unordered_map<std::string,Protocol> stringToProtcolTable{
    {"ls",Protocol::COM_LIST_NG},
    {"NEW",Protocol::COM_CREATE_NG},
    {"DEL",Protocol::COM_DELETE_NG},
    {"lsarticles",Protocol::COM_LIST_ART},
    {"newart",Protocol::COM_CREATE_ART},
    {"delart",Protocol::COM_DELETE_ART},
    {"READ",Protocol::COM_GET_ART},
    {"END",Protocol::COM_END},
    
    {"ANS_LIST_NG",Protocol::ANS_LIST_NG},
    {"ANS_CREATE_NG",Protocol::ANS_CREATE_NG},
    {"ANS_DELETE_NG",Protocol::ANS_DELETE_NG},
    {"ANS_LIST_ART",Protocol::ANS_LIST_ART},
    {"ANS_CREATE_ART",Protocol::ANS_CREATE_ART},
    {"ANS_DELETE_ART",Protocol::ANS_DELETE_ART},
    {"ANS_GET_ART",Protocol::ANS_GET_ART},
    {"ANS_END",Protocol::ANS_END},
    {"ANS_ACK",Protocol::ANS_ACK},
    {"ANS_NAK",Protocol::ANS_NAK},
    
    {"string",Protocol::PAR_STRING},
    {"number",Protocol::PAR_NUM},
    
    {"ERR_NG_ALREADY_EXISTS",Protocol::ERR_NG_ALREADY_EXISTS},
    {"ERR_NG_DOES_NOT_EXIST",Protocol::ERR_NG_DOES_NOT_EXIST},
    {"ERR_ART_DOES_NOT_EXIST",Protocol::ERR_ART_DOES_NOT_EXIST}
    };
};

        
#endif // MESSAGEHANDLER_H_INCLUDED
