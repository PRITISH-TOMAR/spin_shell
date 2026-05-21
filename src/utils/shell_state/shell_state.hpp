#pragma once
#include <vector>
#include <string>
#include <iostream>
using namespace std;

struct ShellState
{
    int lastExitCode;
    vector<string> commandHistory;
    string currentDirectory;
    ShellState() : lastExitCode(0) {}

    int setStateToStatusCode(int code)
    {
        lastExitCode = code;
        return code;
    }

    void printExitStatus() const
    {
        cout << lastExitCode << "\n";
    }

    void expandStatusCode(string &input) const
    {
        size_t pos;
        while ((pos = input.find("$?")) != string::npos)
            input.replace(pos, 2, to_string(lastExitCode));
    }
};