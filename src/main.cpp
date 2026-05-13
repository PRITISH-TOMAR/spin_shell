#include <iostream>
#include <string>
#include "commands/commands.hpp"
#include "utils/parser/parser.hpp"
#include "commands/handlers/echo.hpp"
#include "commands/handlers/help.hpp"
#include "commands/handlers/clear.hpp"
#include "commands/handlers/type.hpp"

using namespace std;

int main() {
    cout << unitbuf;
    cerr << unitbuf;

    string input;

    while (true) {
        cout << "$ ";

        if (!getline(cin, input)) break;
        if (input.empty()) continue;

        ParsedInput parsed = parseInput(input);

        Command cmd = Command::UNKNOWN;
        auto it = commandMap.find(parsed.command);
        if (it != commandMap.end()) cmd = it->second;

        switch (cmd) {
            case Command::EXIT:  return 0;
            case Command::HELP:  handleHelp();               break;
            case Command::CLEAR: handleClear();              break;
            case Command::ECHO:  handleEcho(parsed.args);    break;
            case Command::TYPE:  handleType(parsed.args);    break;
            case Command::UNKNOWN:
                cout << parsed.command << ": command not found\n";
                break;
        }
    }
}