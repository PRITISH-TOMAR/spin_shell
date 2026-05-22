#include "cd.hpp"
#include <iostream>
#include <unistd.h> // chdir, getcwd
#include <limits.h> // PATH_MAX

static string resolveCurrentDirectory()
{
    char buff[PATH_MAX];

    return (getcwd(buff, sizeof(buff)) ? string(buff) : string());
}

int handleCd(const vector<string> &rawArgs, ShellState &state)
{
    string targetDir;

    if (rawArgs.empty())
    {
        const char *home = getenv("HOME");

        if (!home)
        {
            cerr << "cd:HOME not set\n";
            return state.recordCommandExitCode(1);
        }
        targetDir = home;
    }
    else if (rawArgs.size() > 1)
    {
        cerr << "cd: too many arguments\n";
        return state.recordCommandExitCode(1);
    }
    else if (rawArgs[0] == "~")
    {
        const char *home = getenv("HOME");
        if (!home)
        {
            cerr << "cd:HOME not set\n";
            return state.recordCommandExitCode(1);
        }
        targetDir = home;
    }
    else if (rawArgs[0].size() > 1 && rawArgs[0][0] == '~' && rawArgs[0][1] == '/')
    {
        const char *home = getenv("HOME");
        if (!home)
        {
            cerr << "cd:HOME not set\n";
            return state.recordCommandExitCode(1);
        }
        targetDir = string(home) + rawArgs[0].substr(1);
    }
    else if (rawArgs[0] == "-")
    {
        if (state.previousDirectory.empty())
        {
            cerr << "cd: OLDPWD not set\n";
            return state.recordCommandExitCode(1);
        }
        targetDir = state.previousDirectory;
    }
    else
    {
        targetDir = rawArgs[0];
    }

    if (chdir(targetDir.c_str()) != 0)
    {
        cerr << "cd: " << targetDir << ": No such file or directory\n";
        return state.recordCommandExitCode(1);
    }

    string newDir = resolveCurrentDirectory();
    state.previousDirectory = state.currentDirectory;
    state.currentDirectory = newDir;

    if (!rawArgs.empty() && rawArgs[0] == "-")
        cout << newDir << "\n";

    return state.recordCommandExitCode(0);
}