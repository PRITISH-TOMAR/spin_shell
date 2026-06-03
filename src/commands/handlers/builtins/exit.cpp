#include "exit.hpp"
#include <iostream>
#include <cstdlib>

using namespace std;

[[noreturn]] void handleExit(const ParsedInput& parsed, ShellState& state)
{
    int code = state.lastExitCode;

    if (!parsed.rawArgs.empty()) {
        const string& arg = parsed.rawArgs[0];
        try {
            code = stoi(arg) & 0xFF;
        } catch (const exception&) {
            cerr << "exit: " << arg << ": numeric argument required\n";
            code = 2;
        }
    }

    cout << "See Ya! Bye\n";
    exit(code);
}
