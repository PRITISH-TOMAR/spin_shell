#pragma once
#include <string>
#include <unordered_map>

using namespace std;

// Enum definition
enum class Command {
    EXIT,
    HELP,
    CLEAR,
    ECHO,
    UNKNOWN
};

// Declare the map (defined in commands.cpp)
extern unordered_map<string, Command> commandMap;