#pragma once
#include <string>
#include <unordered_map>

using namespace std;

enum class Command {
    // ── Builtins ──────────────────
    EXIT,
    HELP,
    TYPE,
    HISTORY,

    // ── Builtins (affect shell state) ──
    CD,
    PWD,
    EXPORT,

    // ── Tools (unix independent) ───
    LS,
    CAT,
    ECHO,
    GREP,
    MKDIR,
    RM,
    CP,
    MV,
    TOUCH,
    CLEAR,

    UNKNOWN
};

extern unordered_map<string, Command> commandMap;
extern unordered_map<Command, string> commandNames;