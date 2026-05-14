#include "type.hpp"
#include "../../commands/commands.hpp"
#include "../../utils/findInPath.hpp"
#include <iostream>
#include <cstdlib>
#include <filesystem>

namespace fs = filesystem;

static bool isBuiltin(const string &name)
{
    auto commandName = commandMap.find(name);
    return commandName != commandMap.end();
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