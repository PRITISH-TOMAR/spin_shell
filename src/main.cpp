#include <iostream>
#include <string>
#include "commands/commands.hpp"
#include "commands/dispatch.hpp"
#include "utils/executors/executor.hpp"
#include "utils/shell_state/shell_state.hpp"
#include "utils/handlers/input_handler.hpp"
#include "commands/handlers/builtins/exit.hpp"
#include "utils/pipe/pipe_utils.hpp"
#include "utils/pipe/pipe_runner.hpp"
#include "utils/redirections/redirection_guard.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

int main()
{
    cout << unitbuf;
    cerr << unitbuf;

// Enable ANSI escape codes on Windows
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode))
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    ShellState state;
    string input;

    while (true)
    {
        cout << state.getPrompt();

        if (!getline(cin, input))
            return state.lastExitCode;
        if (input.empty())
            continue;

        if (containsPipe(input))
        {
            vector<string> segments = splitOnPipe(input);
            runPipeline(segments, state);
            continue;
        }

        state.commandHistory.push_back(input);

        ParsedInput parsed;
        if (prepareInputForDispatch(input, state, parsed))
            continue;

        Command cmd = Command::UNKNOWN;
        auto it = commandMap.find(parsed.command);
        if (it != commandMap.end())
            cmd = it->second;

        if (cmd == Command::EXIT)
            handleExit(parsed, state);

        RedirectionGuard guard;
        guard.apply(parsed.redirections);

        if (cmd == Command::UNKNOWN)
            state.recordCommandExitCode(executeExternalCommand(parsed.command, parsed.rawArgs));
        else
            dispatchCommand(cmd, parsed, state);
    }
}