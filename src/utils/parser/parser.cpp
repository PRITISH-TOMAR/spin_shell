#include "parser.hpp"
#include <sstream>

ParsedInput parseInput(const string &input)
{
    ParsedInput parsed;
    istringstream stream(input);
    string token;

    if (!(stream >> parsed.command))
    {
        return parsed; // Return empty if no command is found
    }

    while (stream >> token) /// auto split by whitespace
    {
        parsed.rawArgs.push_back(token);
    }

    if (token.size() > 1 && token[0] == '-' && token[1] != '-')
    {
        for (size_t i = 1; i < token.size(); i++)
        {
            parsed.shortFlags[token[i]] = true;
        }
    }
    else if (token.size() > 2 && token.substr(0, 2) == "--")
    {
        string flagName = token.substr(2);
        parsed.longFlags[flagName] = true;
    }
    else
    {
        parsed.files.push_back(token);
    }
    return parsed;
}

// input: "cat -nb --number file.txt ../other.txt"
//            │
//            ▼
// stream >> token  →  command = "cat"
//            │
//            ▼
// stream >> token  →  token = "-nb"
//     token[0] = '-', token[1] = 'n' (not '-')  →  SHORT FLAG
//     loop: shortFlags['n']=true, shortFlags['b']=true
//            │
//            ▼
// stream >> token  →  token = "--number"
//     token[0] = '-', token[1] = '-'  →  LONG FLAG
//     substr(2) → longFlags["number"]=true
//            │
//            ▼
// stream >> token  →  token = "file.txt"
//     doesn't start with '-'  →  FILE
//     files = ["file.txt"]
//            │
//            ▼
// stream >> token  →  token = "../other.txt"
//     doesn't start with '-'  →  FILE
//     files = ["file.txt", "../other.txt"]
//            │
//            ▼
// stream exhausted → loop ends


// Final ParsedInput:
// {
//     command    = "cat"
//     rawArgs    = ["-nb", "--number", "file.txt", "../other.txt"]
//     shortFlags = { 'n':true, 'b':true }
//     longFlags  = { "number":true }
//     files      = ["file.txt", "../other.txt"]
// }



// determine if a flag is present in the parsed input
bool hasFlag(const ParsedInput& parsed, char s, const string& l) {
    return parsed.shortFlags.count(s) || parsed.longFlags.count(l);
}
