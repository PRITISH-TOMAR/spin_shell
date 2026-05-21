#include <iostream>
#include <string>
#include "commands/commands.hpp"
#include "utils/parser/parser.hpp"
#include "utils/executors/executor.hpp"
#include "src/commands/handlers/tools/cat.hpp"
using namespace std;

int main() {
    cout << unitbuf;
    cerr << unitbuf;

    string input;

    while (true) {
        cout << "$ ";

        if (!getline(cin, input)) {
            cout << "EOF \n";
            break;
        };
        if (input.empty()) continue;

        ParsedInput parsed = parseInput(input);

        Command cmd = Command::UNKNOWN;
        auto it = commandMap.find(parsed.command);
        if (it != commandMap.end()) cmd = it->second;

        switch (cmd) {
            case Command::EXIT: return 0;
            case Command::CAT:  handleCat(parsed); break;
            case Command::UNKNOWN:
                executeExternalCommand(parsed.command, parsed.rawArgs);
                break;
            default: break;
        }
    }
}