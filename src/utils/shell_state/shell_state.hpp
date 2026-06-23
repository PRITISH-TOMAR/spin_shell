#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <cstdlib>
using namespace std;

struct ShellState
{
    int lastExitCode;
    bool isPiped = false;
    vector<string> commandHistory;
    string currentDirectory;
    string previousDirectory;
    unordered_map<string, string> variables;

    ShellState();

    int recordCommandExitCode(int code)
    {
        lastExitCode = code;
        return code;
    }

    void displayLastCommandExitCode() const
    {
        cout << lastExitCode << "\n";
    }

    // looks up a variable: shell vars take priority over env vars
    string lookupVariableValue(const string &name) const
    {
        auto it = variables.find(name);
        if (it != variables.end())
            return it->second;

        const char *env = getenv(name.c_str());
        return env ? env : "";
    }

    // expands $? and $var in-place before parsing
    void expandShellVariablesInPlace(string &input) const
    {
        size_t pos = 0;
        while ((pos = input.find('$', pos)) != string::npos)
        {
            // $? — exit status
            if (pos + 1 < input.size() && input[pos + 1] == '?')
            {
                string val = to_string(lastExitCode);
                input.replace(pos, 2, val);
                pos += val.size();
                continue;
            }

            // $name — shell var or env var
            size_t end = pos + 1;
            while (end < input.size() && (isalnum(input[end]) || input[end] == '_'))
                end++;

            string varName = input.substr(pos + 1, end - pos - 1);
            if (varName.empty())
            {
                pos++;
                continue;
            }

            string value = lookupVariableValue(varName);
            input.replace(pos, end - pos, value);
            pos += value.size();
        }
    }

    // returns true if input was a var assignment (x=value), stores it
    bool parseAndStoreVariableAssignment(const string &command)
    {
        size_t eq = command.find('=');
        if (eq == string::npos || eq == 0)
            return false;

        string name = command.substr(0, eq);
        for (char c : name)
            if (!isalnum(c) && c != '_')
                return false;

        variables[name] = command.substr(eq + 1);
        return true;
    }

    string getPrompt() const
    {
        const char *home = getenv("HOME");

        if (home)
        {
            string homeStr(home);

            if (currentDirectory.find(homeStr) == 0)
            { // starts with HOME
                return "~" + currentDirectory.substr(homeStr.size()) + "$ ";
            }
        }
        return currentDirectory + "$ ";
    }
};