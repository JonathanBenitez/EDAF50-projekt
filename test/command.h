#ifndef COMMAND_H
#define COMMAND_H
#include<string>

using std::string;

class Command_1{
	public:
		Command_1(int n, string i = "");

		Command_1& operator=(const Command_1& rhs);

		bool operator!=(const Command_1& rhs);

		bool operator==(const Command_1& rhs);

		int getInt();

		string getInfo();
		
		string getString();
	private:

		string info;
		int nbr;

};

class Command_2{
        public:
                Command_2(int n, string i = "");

                Command_2& operator=(const Command_2& rhs);

                bool operator!=(const Command_2& rhs);

                bool operator==(const Command_2& rhs);

                int getInt();

                string getInfo();

                string getString();
        private:

                string info;
                int nbr;

};


#endif
