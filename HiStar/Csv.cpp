#include "Csv.h"
// endofline: check for and consume \r, \n, \r\n, or EOF
int Csv::endofline(char c)
{
    int eol;

    eol = (c=='\r' || c=='\n');
    if (c == '\r')
    {
        fin.get(c);
        if (!fin.eof() && c != '\n')
            fin.putback(c); // read too far
    }
    return eol;
}

// getline: get one line, grow as needed
int Csv::getline(std::string& str)
{ 
    char c;

    for (line = ""; fin.get(c) && !endofline(c); )
        line += c;
    split();
    str = line;
    return !fin.eof();
}

// split: split line into fields
int Csv::split()
{
    std::string fld;
    int i, j;

    nfield = 0;
    if (line.length() == 0)
        return 0;
    i = 0;

    do {
        if (i < line.length() && line[i] == '"')
            j = advquoted(line, fld, ++i); // skip quote
        else
            j = advplain(line, fld, i);
        if (nfield >= field.size())
            field.push_back(fld);
        else
            field[nfield] = fld;
        nfield++;
        i = j + 1;
    } while (j < line.length());

    return nfield;
}

// advquoted: quoted field; return index of next separator
int Csv::advquoted(const std::string& s, std::string& fld, int i)
{
    int j;

    fld = "";
    for (j = i; j < s.length(); j++)
    {
        if (s[j] == '"' && s[++j] != '"')
        {
            int k = s.find_first_of(fieldsep, j);
            if (k > s.length()) // no separator found
                k = s.length();
            for (k -= j; k-- > 0; )
                fld += s[j++];
            break;
        }
        fld += s[j];
    }
    return j;
}

// advplain: unquoted field; return index of next separator
int Csv::advplain(const std::string& s, std::string& fld, int i)
{
    int j;

    j = s.find_first_of(fieldsep, i); // look for separator
    if (j > s.length()) // none found
        j = s.length();
    fld = std::string(s, i, j-i);
    return j;
}


// getfield: return n-th field
std::string Csv::getfield(int n)
{
    if (n < 0 || n >= nfield)
        return "";
    else
        return field[n];
}
/*
// Csvtest main: test Csv class
int main(void)
{
    string line;
    Csv csv;

    while (csv.getline(line) != 0)
    {
        cout << "line = `" << line <<"'\n";
        for (int i = 0; i < csv.getnfield(); i++)
            cout << "field[" << i << "] = `"
            << csv.getfield(i) << "'\n";
    }
    return 0;
}
*/
