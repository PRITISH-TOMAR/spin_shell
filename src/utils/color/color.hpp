#pragma once
#include <string>
#include <string_view>
#include <filesystem>
#include <regex>

#ifdef _WIN32
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif

using namespace std;
namespace fs = filesystem;

namespace Color {


// is stdout a terminal? (if not, don't output color codes)
inline bool enabled() {
    return ISATTY(FILENO(stdout)) != 0;
}

constexpr string_view RESET   = "\033[0m";
constexpr string_view BOLD    = "\033[1m";
constexpr string_view DIM     = "\033[2m";
constexpr string_view RED     = "\033[31m";
constexpr string_view GREEN   = "\033[32m";
constexpr string_view YELLOW  = "\033[33m";
constexpr string_view BLUE    = "\033[34m";
constexpr string_view MAGENTA = "\033[35m";
constexpr string_view CYAN    = "\033[36m";

// colorize
inline string col(string_view text, string_view code) {
    if (!enabled()) return string(text);
    return string(code) + string(text) + string(RESET);
}

inline string bold(string_view s)    { return col(s, BOLD); }
inline string dim(string_view s)     { return col(s, DIM); }
inline string red(string_view s)     { return col(s, RED); }
inline string green(string_view s)   { return col(s, GREEN); }
inline string yellow(string_view s)  { return col(s, YELLOW); }
inline string blue(string_view s)    { return col(s, BLUE); }
inline string magenta(string_view s) { return col(s, MAGENTA); }
inline string cyan(string_view s)    { return col(s, CYAN); }

inline string boldBlue(string_view s)   { return col(s, string(BOLD) + string(BLUE)); }
inline string boldGreen(string_view s)  { return col(s, string(BOLD) + string(GREEN)); }
inline string boldRed(string_view s)    { return col(s, string(BOLD) + string(RED)); }
inline string boldCyan(string_view s)   { return col(s, string(BOLD) + string(CYAN)); }
inline string boldYellow(string_view s) { return col(s, string(BOLD) + string(YELLOW)); }
inline string dimGreen(string_view s)   { return col(s, string(DIM)  + string(GREEN)); }

// ── Command-specific color functions

string lsColorName(const fs::directory_entry& entry, const fs::file_status& status);

string grepHighlight(const string& line, const regex& re);

string helpHeader(string_view text);

string helpCommand(string_view text);

string helpDesc(string_view text);

string historyLineNum(int n);

string catLineNum(int n);

string typeResult(const string& name, const string& result);

}