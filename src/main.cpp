#include <iostream>
#include <string>
#include "commands/commands.hpp"
#include "utils/executors/executor.hpp"
#include "utils/shell_state/shell_state.hpp"
#include "utils/handlers/input_handler.hpp"
#include "src/commands/handlers/tools/cat.hpp"
#include "src/commands/handlers/tools/echo.hpp"
using namespace std;

int main() {
    cout << unitbuf;
    cerr << unitbuf;

    ShellState state;
    string input;

    while (true) {
        cout << "$ ";

        if (!getline(cin, input)) {
            cout << "EOF \n";
            break;
        }
        if (input.empty()) continue;

        ParsedInput parsed;
        if (prepareInputForDispatch(input, state, parsed)) continue;

        Command cmd = Command::UNKNOWN;
        auto it = commandMap.find(parsed.command);
        if (it != commandMap.end()) cmd = it->second;

        switch (cmd) {
            case Command::EXIT: return 0;
            case Command::CAT:  handleCat(parsed, state); break;
            case Command::ECHO: handleEcho(parsed, state); break;
            case Command::UNKNOWN:
                executeExternalCommand(parsed.command, parsed.rawArgs);
                break;
            default: break;
        }
    }
}