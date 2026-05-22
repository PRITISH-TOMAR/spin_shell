#include "shell_state.hpp"
#include <unistd.h>
#include <limits.h>

ShellState:: ShellState() : lastExitCode(0)
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {
        currentDirectory = cwd;
    }
}