#include "cat.hpp"
#include "src/utils/parser/parser.hpp"
#include "src/utils/shell_state/shell_state.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>


namespace fs = filesystem;

static bool printFileContent(const fs::path &path, bool showLineNumbers)
{
    ifstream file(path);

    if (!file.is_open())
    {
        cerr << "cat: " << path.string() << ": Permission denied\n";
        return false;
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
    return true;
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

int handleCat(const ParsedInput& parsed, ShellState &state){

    if(parsed.rawArgs.empty() || parsed.files.empty()){
        cerr << "cat: missing file operand\n";
        cerr << "Usage: cat [OPTION]... [FILE]...\n";
        return state.setStateToStatusCode(1);
    }

    bool numberAllLines = hasFlag(parsed, 'n', "number");
    bool numberNonEmptyLines = hasFlag(parsed, 'b', "number-nonblank");
    bool showLineNumbers = numberAllLines || numberNonEmptyLines;

    int exitCode = 0;

    for(const string& fileArg: parsed.files){
        fs::path resolved = fs::weakly_canonical(fileArg);

        if(!validatePath(resolved, fileArg) || !printFileContent(resolved, showLineNumbers))
            exitCode = 1;
    }

    return state.setStateToStatusCode(exitCode);
}