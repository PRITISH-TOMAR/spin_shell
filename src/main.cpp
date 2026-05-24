#include <iostream>
#include <string>
#include "commands/commands.hpp"
#include "commands/dispatch.hpp"
#include "utils/executors/executor.hpp"
#include "utils/shell_state/shell_state.hpp"
#include "utils/handlers/input_handler.hpp"
using namespace std;

int main()
{
    cout << unitbuf;
    cerr << unitbuf;

    ShellState state;
    string input;

    while (true)
    {
        cout << state.getPrompt();

        if (!getline(cin, input))
        {
            cout << "EOF \n";
            break;
        }
        if (input.empty())
            continue;

        ParsedInput parsed;
        if (prepareInputForDispatch(input, state, parsed))
            continue;

        Command cmd = Command::UNKNOWN;
        auto it = commandMap.find(parsed.command);
        if (it != commandMap.end())
            cmd = it->second;

        if (cmd == Command::EXIT)
            return 0;

        if (cmd == Command::UNKNOWN)
            executeExternalCommand(parsed.command, parsed.rawArgs);
        else
            dispatchCommand(cmd, parsed, state);
    }
}