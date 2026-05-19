#pragma once
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct ParsedInput {
    string                      command;
    vector<string>              rawArgs;
    vector<string>              files;        // non-flag args
    unordered_map<char, bool>   shortFlags;   // -n -b -a -l
    unordered_map<string, bool> longFlags;    // --number --help
};

ParsedInput parseInput(const string& input);