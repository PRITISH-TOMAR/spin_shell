#include "commands.hpp"

// string → enum (for lookup during input)
unordered_map<string, Command> commandMap = {
    {"exit",  Command::EXIT},
    {"help",  Command::HELP},
    {"clear", Command::CLEAR},
    {"echo",  Command::ECHO},
    {"type",  Command::TYPE},
};

// enum =  string (for display, type checks, help text)
unordered_map<Command, string> commandNames = {
    {Command::EXIT,  "exit"},
    {Command::HELP,  "help"},
    {Command::CLEAR, "clear"},
    {Command::ECHO,  "echo"},
    {Command::TYPE,  "type"},
};