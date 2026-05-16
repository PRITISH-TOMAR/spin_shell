#include "commands.hpp"

unordered_map<string, Command> commandMap = {
    // builtins
    {"exit",    Command::EXIT},
    {"help",    Command::HELP},
    {"type",    Command::TYPE},
    {"history", Command::HISTORY},

    // builtins (shell state)
    {"cd",      Command::CD},
    {"pwd",     Command::PWD},
    {"export",  Command::EXPORT},

    // tools
    {"ls",      Command::LS},
    {"cat",     Command::CAT},
    {"echo",    Command::ECHO},
    {"grep",    Command::GREP},
    {"mkdir",   Command::MKDIR},
    {"rm",      Command::RM},
    {"cp",      Command::CP},
    {"mv",      Command::MV},
    {"touch",   Command::TOUCH},
    {"clear",   Command::CLEAR},
};

unordered_map<Command, string> commandNames = {
    {Command::EXIT,    "exit"},
    {Command::HELP,    "help"},
    {Command::TYPE,    "type"},
    {Command::HISTORY, "history"},
    {Command::CD,      "cd"},
    {Command::PWD,     "pwd"},
    {Command::EXPORT,  "export"},
    {Command::LS,      "ls"},
    {Command::CAT,     "cat"},
    {Command::ECHO,    "echo"},
    {Command::GREP,    "grep"},
    {Command::MKDIR,   "mkdir"},
    {Command::RM,      "rm"},
    {Command::CP,      "cp"},
    {Command::MV,      "mv"},
    {Command::TOUCH,   "touch"},
    {Command::CLEAR,   "clear"},
};