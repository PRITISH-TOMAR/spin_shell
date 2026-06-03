#include "history.hpp"
#include "src/utils/handlers/flag_set.hpp"
#include <iostream>
#include <stdexcept>

using namespace std;

int handleHistory(const ParsedInput& parsed, ShellState& state)
{
    FlagSet flags(HISTORY_FLAGS);
    flags.parse(parsed);

    // -c: clear history
    if (flags.has('c')) {
        state.commandHistory.clear();
        return 0;
    }

    // -d offset: delete entry at 1-based position
    string dval = flags.value('d');
    if (!dval.empty()) {
        try {
            int pos = stoi(dval);
            auto& hist = state.commandHistory;
            if (pos < 1 || (size_t)pos > hist.size()) {
                cerr << "history: " << pos << ": history position out of range\n";
                return 1;
            }
            hist.erase(hist.begin() + (pos - 1));
        } catch (const exception&) {
            cerr << "history: " << dval << ": numeric argument required\n";
            return 1;
        }
        return 0;
    }

    // history [N]: list all or last N entries
    const auto& hist = state.commandHistory;
    size_t start = 0;

    if (!parsed.files.empty()) {
        try {
            int n = stoi(parsed.files[0]);
            if (n < 0) {
                cerr << "history: " << parsed.files[0] << ": invalid option\n";
                return 1;
            }
            if ((size_t)n < hist.size())
                start = hist.size() - n;
        } catch (const exception&) {
            cerr << "history: " << parsed.files[0] << ": numeric argument required\n";
            return 1;
        }
    }

    for (size_t i = start; i < hist.size(); ++i)
        cout << "  " << (i + 1) << "  " << hist[i] << "\n";

    return 0;
}
