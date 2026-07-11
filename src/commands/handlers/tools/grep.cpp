#include "grep.hpp"
#include "src/utils/path/path.hpp"
#include "src/utils/handlers/flag_set.hpp"
#include "src/utils/color/color.hpp"
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

struct GrepOptions {
    bool ignoreCase = false;
    bool lineNumber = false;
    bool recursive  = false;
    bool filesOnly  = false;
    bool countOnly  = false;
    bool invert     = false;
};

// Escapes all regex metacharacters so the pattern is treated as a literal string.
static string regexEscape(const string& s) {
    static const string meta = R"(\.^$*+?()[]{}|)";
    string result;
    for (char c : s) {
        if (meta.find(c) != string::npos) result += '\\';
        result += c;
    }
    return result;
}

static bool grepStream(istream& in, const regex& re, const string& label,
                       bool printLabel, const GrepOptions& opts)
{
    string line;
    int lineNum = 0;
    int matchCount = 0;
    bool found = false;

    while (getline(in, line)) {
        ++lineNum;
        bool matches = regex_search(line, re);
        if (opts.invert) matches = !matches;

        if (matches) {
            found = true;
            ++matchCount;
            if (opts.filesOnly) {
                cout << label << "\n";
                return true;
            }
            if (!opts.countOnly) {
                if (printLabel)      cout << Color::magenta(label) << ":";
                if (opts.lineNumber) cout << Color::green(to_string(lineNum)) << ":";
                cout << Color::grepHighlight(line, re) << "\n";
            }
        }
    }

    if (opts.countOnly) {
        if (printLabel) cout << label << ":";
        cout << matchCount << "\n";
    }

    return found;
}

static bool grepFile(const fs::path& path, const string& display,
                     const regex& re, bool printLabel, const GrepOptions& opts)
{
    error_code ec;
    if (!fs::exists(path, ec)) {
        cerr << "grep: " << display << ": No such file or directory\n";
        return false;
    }
    ifstream f(path);
    if (!f) {
        cerr << "grep: " << display << ": Permission denied\n";
        return false;
    }
    return grepStream(f, re, display, printLabel, opts);
}

static bool grepDir(const fs::path& dir, const regex& re, const GrepOptions& opts) {
    bool found = false;
    error_code ec;
    for (auto& entry : fs::recursive_directory_iterator(
             dir, fs::directory_options::skip_permission_denied, ec))
    {
        if (entry.is_regular_file(ec)) {
            string p = entry.path().string();
            if (grepFile(entry.path(), p, re, true, opts))
                found = true;
        }
    }
    return found;
}

int handleGrep(const ParsedInput& parsed, ShellState& state) {
    FlagSet flags(GREP_FLAGS);
    flags.parse(parsed);

    GrepOptions opts;
    opts.ignoreCase = flags.has('i');
    opts.lineNumber = flags.has('n');
    opts.recursive  = flags.has('r') || flags.has('R');
    opts.filesOnly  = flags.has('l');
    opts.countOnly  = flags.has('c');
    opts.invert     = flags.has('v');

    if (parsed.files.empty()) {
        cerr << "grep: missing pattern\n";
        cerr << "Usage: grep [OPTION]... PATTERN [FILE...]\n";
        return state.recordCommandExitCode(2);
    }

    const string& pattern = parsed.files[0];

    auto reFlags = regex::ECMAScript | regex::optimize;
    if (opts.ignoreCase) reFlags |= regex::icase;

    regex re;
    try {
        re = regex(pattern, reFlags);
    } catch (const regex_error&) {
        // Pattern is not valid regex — fall back to literal string match
        re = regex(regexEscape(pattern), reFlags);
    }

    bool found = false;

    if (parsed.files.size() == 1) {
        // No file args — read from stdin
        found = grepStream(cin, re, "(standard input)", false, opts);
    } else {
        vector<string> targets(parsed.files.begin() + 1, parsed.files.end());
        bool multiTarget = targets.size() > 1 || opts.recursive;

        for (const string& t : targets) {
            fs::path p = resolvePath(t);
            error_code ec;
            if (fs::is_directory(p, ec)) {
                if (opts.recursive) {
                    if (grepDir(p, re, opts)) found = true;
                } else {
                    cerr << "grep: " << t << ": Is a directory\n";
                }
            } else {
                if (grepFile(p, t, re, multiTarget, opts)) found = true;
            }
        }
    }

    return state.recordCommandExitCode(found ? 0 : 1);
}
