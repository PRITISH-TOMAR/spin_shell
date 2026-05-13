#include "help.hpp"
#include "../../commands/commands.hpp"
#include <iostream>

void handleHelp() {
    cout << "Available commands:\n";

    // Uses commandNames map — no hardcoding!
    for (const auto& [cmd, name] : commandNames) {
        if (cmd != Command::UNKNOWN) {
            cout << "  " << name << "\n";
        }
    }
}