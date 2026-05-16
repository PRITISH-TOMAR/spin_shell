#include "cd.hpp"
#include <iostream>
#include <unistd.h>      // Linux
// #include <direct.h>   // Windows → _chdir()

void handleCd(const vector<string>& rawArgs) {
    if (rawArgs.empty()) {
        // cd with no args → go home
        const char* home = getenv("HOME");
        if (!home) { cerr << "cd: HOME not set\n"; return; }
        chdir(home);
        return;
    }

    if (chdir(rawArgs[0].c_str()) != 0) {
        cerr << "cd: " << rawArgs[0] << ": No such file or directory\n";
    }
}