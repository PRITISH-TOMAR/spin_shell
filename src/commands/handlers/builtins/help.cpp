#include "help.hpp"
#include "src/utils/color/color.hpp"
#include <iostream>

using namespace std;

int handleHelp(const ParsedInput& parsed, ShellState& state) {
    struct CmdInfo { const char* name; const char* desc; };
    static const CmdInfo cmds[] = {
        {"cat",     "Concatenate and print files"},
        {"cd",      "Change the current directory"},
        {"clear",   "Clear the terminal screen"},
        {"cp",      "Copy files and directories"},
        {"echo",    "Display a line of text"},
        {"exit",    "Exit the shell"},
        {"export",  "Set an environment variable"},
        {"grep",    "Search for patterns in files or stdin"},
        {"help",    "Display this help message"},
        {"history", "Show or manage command history"},
        {"ls",      "List directory contents"},
        {"mkdir",   "Create directories"},
        {"mv",      "Move or rename files and directories"},
        {"pwd",     "Print the current working directory"},
        {"rm",      "Remove files or directories"},
        {"rmdir",   "Remove empty directories"},
        {"touch",   "Create files or update timestamps"},
        {"type",    "Describe how a command would be interpreted"},
    };

    cout << Color::helpHeader("Shell built-in commands:") << "\n\n";
    for (const auto& cmd : cmds) {
        // Pad the plain name to 12 chars before colorizing so alignment is preserved
        string padded = string(cmd.name) + string(12 - strlen(cmd.name), ' ');
        cout << "  " << Color::helpCommand(padded) << Color::helpDesc(cmd.desc) << "\n";
    }

    return state.recordCommandExitCode(0);
}
