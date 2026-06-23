#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "src/utils/redirections/redirection.hpp" 

using namespace std;

struct ParsedInput {
    string                      command;
    vector<string>              rawArgs;
    vector<bool>                rawArgsHasGlob; // parallel to rawArgs: true if token had unquoted * ? [
    vector<string>              files;          // non-flag args
    unordered_map<char, bool>   shortFlags;     // -n -b -a -l
    unordered_map<string, bool> longFlags;      // --number --help
    vector<Redirection> redirections; // parsed redirections
};

ParsedInput parseInput(const string& input);
bool hasFlag(const ParsedInput& parsed, char s, const string& l);