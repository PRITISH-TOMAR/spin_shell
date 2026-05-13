#include "command.hpp"

// Define the map here
unordered_map<string, Command> commandMap = {
    {"exit",  Command::EXIT},
    {"help",  Command::HELP},
    {"clear", Command::CLEAR},
    {"echo",  Command::ECHO},
};