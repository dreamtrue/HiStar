/* Copyright (C) 1999 Lucent Technologies */
/* Excerpted from 'The Practice of Programming' */
/* by Brian W. Kernighan and Rob Pike */

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

//using namespace std;

class Csv
{ // read and parse comma-separated values
    // sample input: "LU",86.25,"11/4/1998","2:19PM",+4.0625

public:
    Csv(std::istream& fin = std::cin, std::string sep = ",") : 
      fin(fin), fieldsep(sep) {}

      int getline(std::string&);
      std::string getfield(int n);
      int getnfield() const { return nfield; }

private:
    std::istream& fin; // input file pointer
    std::string line; // input line
    std::vector<std::string> field; // field strings
    int nfield; // number of fields
    std::string fieldsep; // separator characters

    int split();
    int endofline(char);
    int advplain(const std::string& line, std::string& fld, int);
    int advquoted(const std::string& line, std::string& fld, int);
};

