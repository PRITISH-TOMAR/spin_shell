#include "echo.hpp"
#include <iostream>

using namespace std;

int handleEcho(const ParsedInput &parsed, ShellState &state)
{
    bool noNewline = parsed.shortFlags.count('n') && parsed.shortFlags.at('n');

    for (size_t i = 0; i < parsed.files.size(); ++i)
    {
        if (i > 0) cout << " ";
        cout << parsed.files[i];
    }

    if (!noNewline) cout << "\n";

    return state.recordCommandExitCode(0);
}
