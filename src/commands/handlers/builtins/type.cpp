#include "type.hpp"
#include "src/commands/commands.hpp"
#include "src/utils/findInPath.hpp"
#include "src/utils/color/color.hpp"
#include <iostream>

int handleType(const ParsedInput& parsed, ShellState& state) {
    if (parsed.files.empty()) {
        cerr << "type: missing argument\n";
        return state.recordCommandExitCode(1);
    }

    int exitCode = 0;
    for (const string& name : parsed.files) {
        if (commandMap.count(name)) {
            cout << Color::typeResult(name, "builtin") << "\n";
        } else {
            string path = findInPath(name);
            if (!path.empty()) {
                cout << Color::typeResult(name, path) << "\n";
            } else {
                cerr << Color::typeResult(name, "") << "\n";
                exitCode = 1;
            }
        }
    }
    return state.recordCommandExitCode(exitCode);
}
