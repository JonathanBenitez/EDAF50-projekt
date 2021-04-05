#include"command.h"
#include<string>

using std::string;

Command_1::Command_1(int n, string i){
nbr = n;
info = i;
}

Command_1& Command_1::operator=(const Command_1& rhs){
nbr = rhs.nbr;
info = rhs.info;

return *this;
}

bool Command_1::operator!=(const Command_1& rhs){
return info!=rhs.info||nbr!=rhs.nbr;
}

bool Command_1::operator==(const Command_1& rhs){
return info==rhs.info && nbr==rhs.nbr;
}

int Command_1::getInt(){
return nbr;
}

string Command_1::getString(){
return info;
}

string Command_1::getInfo(){
return info + std::to_string(nbr);
}

Command_2::Command_2(int n, string i){
nbr = n;
info = i;
}

Command_2& Command_2::operator=(const Command_2& rhs){
nbr = rhs.nbr;
info = rhs.info;

return *this;
}

bool Command_2::operator!=(const Command_2& rhs){
return info!=rhs.info||nbr!=rhs.nbr;
}

bool Command_2::operator==(const Command_2& rhs){
return info==rhs.info && nbr==rhs.nbr;
}

int Command_2::getInt(){
return nbr;
}

string Command_2::getString(){
return info;
}

string Command_2::getInfo(){
return info + std::to_string(nbr);
}

