#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "src/utils/parser/parser.hpp"
#include "src/utils/handlers/flag_value.hpp"

using namespace std;

enum class FlagKind { Bool, Value };

struct FlagDef {
    char   shortName;
    string longName;
    FlagKind kind;
};

// ── Global flag registry ──────────────────────────────────────────────────────
// ALL supported flags for every command live here.
// When adding a new command that uses flags, add its named section below.
// Handlers reference only their named set — no flag definitions inside handlers.

inline const vector<FlagDef> TOUCH_FLAGS = {
    {'a', "",           FlagKind::Bool},   // update only access time
    {'m', "",           FlagKind::Bool},   // update only modification time
    {'c', "no-create",  FlagKind::Bool},   // skip file if it doesn't exist
    {'d', "date",       FlagKind::Value},  // datetime string e.g. "2024-01-01 12:00"
    {'t', "",           FlagKind::Value},  // timestamp [[CC]YY]MMDDhhmm[.ss]
    {'r', "reference",  FlagKind::Value},  // use timestamps from reference file
};

inline const vector<FlagDef> MKDIR_FLAGS = {
    {'p', "parents", FlagKind::Bool},   // create parent directories
    {'v', "verbose", FlagKind::Bool},   // print message for each created dir
    {'m', "mode",    FlagKind::Value},  // octal permission bits e.g. "755"
};

inline const vector<FlagDef> RM_FLAGS = {
    {'r', "recursive", FlagKind::Bool},  // remove directories and their contents recursively
    {'f', "force",     FlagKind::Bool},  // ignore nonexistent files and arguments, never prompt
    {'i', "interactive", FlagKind::Bool},  // prompt before every removal
    {'v', "verbose",     FlagKind::Bool},  // print message for each removed file
};

inline const vector<FlagDef> RMDIR_FLAGS = {
    {'p', "parents", FlagKind::Bool},  // remove path components, each must be empty
    {'v', "verbose", FlagKind::Bool},  // print message for each removed directory
};

inline const vector<FlagDef> HISTORY_FLAGS = {
    {'c', "", FlagKind::Bool},   // clear history
    {'d', "", FlagKind::Value},  // delete entry at offset (1-based)
};

// Parses a command's declared flags from ParsedInput.
// Construct with a named flag set from the registry above, then call parse().
class FlagSet {
public:
    // explicit: prevents accidental implicit conversion from vector<FlagDef> to FlagSet
    explicit FlagSet(const vector<FlagDef>& defs) : defs_(defs) {}

    void parse(const ParsedInput& parsed) {
        for (const auto& def : defs_) {
            if (def.kind == FlagKind::Value)
                values_[def.shortName] = getFlagValue(parsed.rawArgs, def.shortName, def.longName);
            else
                bools_[def.shortName] = hasFlag(parsed, def.shortName, def.longName);
        }
    }

    bool has(char s) const {
        auto it = bools_.find(s);
        return it != bools_.end() && it->second;
    }

    string value(char s) const {
        auto it = values_.find(s);
        return it != values_.end() ? it->second : "";
    }

    // Returns true if token was consumed as the argument to a value-flag.
    // Use this to skip value tokens when iterating parsed.files.
    bool isValueToken(const string& token) const {
        for (const auto& [s, v] : values_)
            if (!v.empty() && token == v) return true;
        return false;
    }

private:
    vector<FlagDef> defs_;
    unordered_map<char, bool> bools_;
    unordered_map<char, string> values_;
};
