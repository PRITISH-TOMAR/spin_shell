#include <iostream>
#include <string>
#include "./commands/command.hpp"
#include "./parser/parser.hpp"

using namespace std;

int main() {
    cout << unitbuf;
    cerr << unitbuf;

    string input;

    while (true) {
        cout << "$ ";

        if (!getline(cin, input)) break;
        if (input.empty()) continue;

        // 1. Parse the input
        ParsedInput parsed = parseInput(input);

        // 2. Look up command
        Command cmd = Command::UNKNOWN;
        auto it = commandMap.find(parsed.command);
        if (it != commandMap.end()) {
            cmd = it->second;
        }

        // 3. Switch on command
        switch (cmd) {
            case Command::EXIT:
                return 0;

            case Command::HELP:
                cout << "Available commands: exit, help, clear, echo\n";
                break;

            case Command::CLEAR:
                system("clear");
                break;

            case Command::ECHO:
                // join all args and print
                for (const string& arg : parsed.args) {
                    cout << arg << " ";
                }
                cout << "\n";
                break;

            case Command::UNKNOWN:
                cout << parsed.command << ": command not found\n";
                break;
        }
    }
}