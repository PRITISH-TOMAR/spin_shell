#include "clear.hpp"
#include <iostream>

using namespace std;

int handleClear(const ParsedInput& parsed, ShellState& state)
{
    // ANSI: clear screen and move cursor to home
    cout << "\033[2J\033[H";
    return 0;
}
