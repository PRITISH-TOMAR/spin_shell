#include "type.hpp"
#include "../../commands/commands.hpp"
#include <iostream>
#include <cstdlib>
#include <filesystem>

namespace fs = filesystem;

static bool isBuiltin(const string &name)
{
    auto commandName = commandMap.find(name);
    return commandName != commandMap.end();
}

static string findInPath(const string &command)
{

    // get $PATH value
    const char *pathEnv = getenv("PATH");
    if (!pathEnv)
        return "";

    stringstream ss(pathEnv);
    string dir;

    // Handle Windows vs Linux path separators
#ifdef _WIN32
    char delimiter = ';';
#else
    char delimiter = ':';
#endif

    // path string -> chunks dir -> dellimeter
    while (getline(ss, dir, delimiter))
    {
        fs::path full = fs::path(dir) / command;
        if (fs::is_regular_file(full))
        {
            return full.string();
        }

#ifdef _WIN32
        // On Windows, executables often have a .exe extension
        fs::path fullExe = fs::path(dir) / (command + ".exe");
        if (fs::is_regular_file(fullExe))
        {
            return fullExe.string();
        }
#endif
    }

    return "";
}

void handleType(const vector<string> &args)
{
    if (args.empty())
    {
        cout << "type: missing argument\n";
        return;
    }

    for (const string &arg : args)
    {
        if (isBuiltin(arg))
        {
            cout << arg << " is a shell builtin\n";
        }
        else
        {
            string path = findInPath(arg);
            if (!path.empty())
                cout << arg << " is " << path << "\n";
            else
                cout << arg << ": not found\n";
        }
    }
}