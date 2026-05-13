#pragma once
#include <string>
#include <vector>

using namespace std;

// Holds everything parsed from one line of input
struct ParsedInput {
    string         command;   // "echo", "exit", "ls" etc
    vector<string> args;      // ["hello", "world"]
    vector<string> flags;     // ["-l", "-a", "--help"]
};

// Main parser function
ParsedInput parseInput(const string& input);