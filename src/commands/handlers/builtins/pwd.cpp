#include "pwd.hpp"
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std ::filesystem;

int handlePwd(ShellState& state)
{
    cout << fs::current_path().string() << "\n";
    return state.recordCommandExitCode(0);
}