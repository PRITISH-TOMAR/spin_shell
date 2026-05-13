#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

// Enum for all commands
enum class Command {
    EXIT,
    HELP,
    CLEAR,
    ECHO,
    TYPE,
    UNKNOWN
};


extern unordered_map<string, Command> commandMap;


extern unordered_map<Command, string> commandNames;