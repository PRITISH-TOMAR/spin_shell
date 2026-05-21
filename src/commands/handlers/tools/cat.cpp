#include "cat.hpp"
#include "utils/parser/parser.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>


namespace fs = filesystem;

static void printFileContent(const fs ::path &path, bool showLineNumbers)
{
    ifstream file(path);

    if (!file.is_open())
    {
        cerr << "cat: " << path.string() << ": Permission denied\n";
        return;
    }

    string line;
    int lineNumber = 1;

    while (getline(file, line))
    {
        if (showLineNumbers)
        {
            cout.width(6);
            cout << right << lineNumber++ << "  " << line << "\n";
        }
        else
        {
            cout << line << "\n";
        }
    }
}

static bool validatePath(const fs::path& resolved, const string& original){
    if(!fs::exists(resolved)){
        cerr<< "cat: " << original << ": No such file or directory\n";
        return false;
    }
    if(fs::is_directory(resolved)){
        cerr<< "cat: " << original << ": Is a directory\n";
        return false;
    }
    return true;
}

void handleCat(const ParsedInput& parsed){

    if(parsed.rawArgs.empty()){
        cerr << "cat: missing file operand\n";
        cerr << "Usage: cat [OPTION]... [FILE]...\n";
        return;
    }

    // parse flags
    bool numberAllLines = hasFlag(parsed, 'n', "number");
    bool numberNonEmptyLines = hasFlag(parsed, 'b', "number-nonblank");

    bool showLineNumbers = numberAllLines || numberNonEmptyLines;

    if(parsed.files.empty()){
        cerr << "cat: missing file operand\n";
        cerr << "Usage: cat [OPTION]... [FILE]...\n";
        return;
    }

    // process each file in order
    for(const string& fileArg: parsed.files){
        fs:: path resolved = fs::weakly_canonical(fileArg);

        if(!validatePath(resolved, fileArg))continue;

        printFileContent(resolved, showLineNumbers);
    }


}