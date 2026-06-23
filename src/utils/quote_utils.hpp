#pragma once
#include <string>
using namespace std;

// Tracks single/double quote state while walking a string character by character.
//
// Usage:
//   QuoteTracker qt;
//   for (char c : input) {
//       qt.update(c);
//       if (c == '|' && !qt.inQuotes()) { ... }
//   }
struct QuoteTracker {
    bool inSingle = false;
    bool inDouble = false;

    void update(char c) {
        if      (c == '\'' && !inDouble) inSingle = !inSingle;
        else if (c == '"'  && !inSingle) inDouble = !inDouble;
    }

    bool inQuotes() const { return inSingle || inDouble; }
};

// Returns true if position `pos` in `s` is inside a quoted region.
inline bool isInQuotes(const string& s, size_t pos) {
    QuoteTracker qt;
    for (size_t j = 0; j < pos; ++j) qt.update(s[j]);
    return qt.inQuotes();
}
