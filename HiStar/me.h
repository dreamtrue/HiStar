#include <string>
struct sqldb{
	std::string host;
	std::string user;
	std::string passwd;
	std::string db;
	sqldb();
};
struct stock{
	std::string exch;
	std::string code;
	double volume;
};
