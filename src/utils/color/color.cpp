#include "color.hpp"
#include <regex>
#include <iomanip>
#include <sstream>

using namespace std;
namespace fs = filesystem;

namespace Color {

// ── ls ───────────────────────────────────────────────────────────────────────

string lsColorName(const fs::directory_entry& entry, const fs::file_status& status) {
    string name = entry.path().filename().string();

    if (fs::is_symlink(entry.symlink_status()))
        return cyan(name);
    if (fs::is_directory(status))
        return boldBlue(name);

    auto perms = status.permissions();
    bool isExec = (perms & fs::perms::owner_exec)  != fs::perms::none
               || (perms & fs::perms::group_exec)  != fs::perms::none
               || (perms & fs::perms::others_exec) != fs::perms::none;
    if (isExec)
        return boldGreen(name);
    if (!name.empty() && name[0] == '.')
        return dim(name);

    return name;
}

// ── grep ─────────────────────────────────────────────────────────────────────

string grepHighlight(const string& line, const regex& re) {
    if (!enabled()) return line;

    string result;
    auto begin = sregex_iterator(line.begin(), line.end(), re);
    auto end   = sregex_iterator();
    size_t last = 0;

    for (auto it = begin; it != end; ++it) {
        const smatch& m = *it;
        result += line.substr(last, m.position() - last);
        result += boldRed(m.str());
        last = m.position() + m.length();
    }
    result += line.substr(last);
    return result;
}

// ── help ─────────────────────────────────────────────────────────────────────

string helpHeader(string_view text)  { return boldYellow(text); }
string helpCommand(string_view text) { return boldCyan(text); }
string helpDesc(string_view text)    { return dim(text); }

// ── history ──────────────────────────────────────────────────────────────────

string historyLineNum(int n) {
    return dim("  " + to_string(n) + "  ");
}

// ── cat ──────────────────────────────────────────────────────────────────────

string catLineNum(int n) {
    ostringstream oss;
    oss << setw(6) << right << n;
    return dimGreen(oss.str());
}

// ── type ─────────────────────────────────────────────────────────────────────

string typeResult(const string& name, const string& result) {
    if (result == "builtin")
        return name + " is " + cyan("a shell builtin");
    if (!result.empty())
        return name + " is " + green(result);
    return name + ": " + red("not found");
}

} // namespace Color
