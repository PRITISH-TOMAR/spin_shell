#include<sstream>
#include<filesystem>
#include <iostream>
#include <cstdlib>

namespace fs = std::filesystem;

static string findInPath(const string &command)
{

    // get $PATH 
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
