#pragma once
#include <string>
#include <vector>
using std::string;
using std::vector;

// Returns the value token immediately following a flag in rawArgs.
// Matches "-x value" (shortFlag) or "--long-name value" (longFlag).
// Pass '\0' or ' ' as shortFlag / empty string as longFlag to skip that form.
// Returns "" if the flag is not found or has no following token.
inline string getFlagValue(const vector<string>& rawArgs, char shortFlag, const string& longFlag) {
    for (size_t i = 0; i + 1 < rawArgs.size(); i++) {
        const string& a = rawArgs[i];
        if ((shortFlag != '\0' && a == string("-") + shortFlag) ||
            (!longFlag.empty() && a == "--" + longFlag))
            return rawArgs[i + 1];
    }
    return "";
}
