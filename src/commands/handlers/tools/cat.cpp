#include "cat.hpp"
#include "src/utils/parser/parser.hpp"
#include "src/utils/shell_state/shell_state.hpp"
#include "src/utils/path/path.hpp"
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

    if(parsed.files.empty()){
        string line;
        while(getline(cin, line))
            cout << line << "\n";
        return state.recordCommandExitCode(0);
    }

    bool numberAllLines = hasFlag(parsed, 'n', "number");
    bool numberNonEmptyLines = hasFlag(parsed, 'b', "number-nonblank");
    bool showLineNumbers = numberAllLines || numberNonEmptyLines;

    int exitCode = 0;

    for(const string& fileArg: parsed.files){
        fs::path resolved = resolvePath(fileArg);

        if(!validatePath(resolved, fileArg) || !printFileContent(resolved, showLineNumbers))
            exitCode = 1;
    }

    return state.recordCommandExitCode(exitCode);
}