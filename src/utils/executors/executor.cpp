#include "executor.hpp"
#include "../../utils/findInPath.hpp"
#include <iostream>

#ifdef _WIN32
// Windows-specific includes
#include <process.h>
#else
// Linux/MAc path
#include <unistd.h>
#include <sys/wait.h>
#endif

int executeExternalCommand(const string &command, const vector<string> &rawArgs)
{

    string path = findInPath(command);

    if (path.empty())
    {
        cout << command << ": command not found\n";
        return 127;
    }
    vector<const char *> argV;
    argV.push_back(command.c_str()); // convert to c string fmat

    for (const string &arg : rawArgs)
    {
        argV.push_back(arg.c_str());
    }
    argV.push_back(nullptr); // requires a terminator

#ifdef _WIN32
    // Windows-specific process creation
    int result = _spawnvp(_P_WAIT, path.c_str(), argV.data());
    return (result < 0) ? 1 : result;
#else
    //  Linux/Mac process creation
    pid_t pid = fork();

    if (pid < 0)
    {
        cerr << "Fork failed\n";
        return 1;
    }

    if (pid == 0)
    {
        // Child process
        execv(path.c_str(), const_cast<char *const *>(argV.data()));
        exit(1);
    }
    else
    {
        // Parent process waits for child to finish
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
#endif
}