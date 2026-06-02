#include "ls.hpp"
#include "src/utils/path/path.hpp"

#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace fs = filesystem;

struct LsOptions
{
    bool showAll = false;    // -a
    bool longFormat = false; // -l
    bool humanRead = false;  // -h
    bool sortByTime = false; // -t
    bool sortBySize = false; // -S
    bool reverse = false;    // -r
    bool recursive = false;  // -R
    bool onePerLine = false; // -1
    bool dirItself = false;  // -d
};

//  rwxrwxrwx string from the entry's permission bits.
static string formatPermissions(fs::file_status status)
{
    auto p = status.permissions();
    string result = fs::is_directory(status) ? "d" : "-";

    //  '-' for unset permissions, 'r', 'w', 'x' for set permissions.
    auto bit = [&](fs::perms perm, char c)
    {
        result += (p & perm) != fs::perms::none ? c : '-';
    };

    bit(fs::perms::owner_read, 'r');
    bit(fs::perms::owner_write, 'w');
    bit(fs::perms::owner_exec, 'x');
    bit(fs::perms::group_read, 'r');
    bit(fs::perms::group_write, 'w');
    bit(fs::perms::group_exec, 'x');
    bit(fs::perms::others_read, 'r');
    bit(fs::perms::others_write, 'w');
    bit(fs::perms::others_exec, 'x');

    return result;
}

static string humanSize(uintmax_t size)
{
    if (size >= 1024ULL * 1024 * 1024)
        return to_string(size / (1024 * 1024 * 1024)) + "G";
    if (size >= 1024ULL * 1024)
        return to_string(size / (1024 * 1024)) + "M";
    if (size >= 1024ULL)
        return to_string(size / 1024) + "K";
    return to_string(size) + "B";
}

// Returns the last-write time of a path as a time_t.
// Bridges C++ filesystem's file_time_type to the system clock
static time_t fileTime(const fs::path &p)
{
    auto ftime = fs::last_write_time(p);
    auto sctp = chrono::time_point_cast<chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + chrono::system_clock::now());
    return chrono::system_clock::to_time_t(sctp);
}

// Returns file size in bytes; directories always report 0 (no block count).
static uintmax_t fileSize(const fs::directory_entry &e)
{
    if (fs::is_directory(e.status()))
        return 0;
    error_code ec;
    uintmax_t s = fs::file_size(e.path(), ec);
    return ec ? 0 : s;
}

static void printEntry(const fs::directory_entry &entry, const LsOptions &opts)
{
    fs::file_status status = entry.status();
    string name = entry.path().filename().string();
    bool isDir = fs::is_directory(status);

    if (opts.longFormat)
    {
        uintmax_t sz = fileSize(entry);
        // Use human-readable size only for files (directories show 0)
        string sizeStr = opts.humanRead && !isDir ? humanSize(sz) : to_string(sz);

        // Format modification time as "Mon DD HH:MM"
        time_t t = fileTime(entry.path());
        char timeBuf[20];
        strftime(timeBuf, sizeof(timeBuf), "%b %d %H:%M", localtime(&t));

        cout << formatPermissions(status)
             << "  " << setw(8) << right << sizeStr
             << "  " << timeBuf
             << "  " << name;
        if (isDir)
            cout << "/";
        cout << "\n";
    }
    else if (opts.onePerLine)
    {
        // -1: each entry on its own line, no padding
        cout << name;
        if (isDir)
            cout << "/";
        cout << "\n";
    }
    else
    {
        // Default short mode: entries separated by two spaces
        cout << name;
        if (isDir)
            cout << "/";
        cout << "  ";
    }
}

// Forward-declare so the recursive call inside the function body compiles.
static void listDirectory(const fs::path &dir, const LsOptions &opts, bool isTopLevel = true);

static void listDirectory(const fs::path &dir, const LsOptions &opts, bool isTopLevel)
{
    vector<fs::directory_entry> entries;

    // Collect entries, skipping hidden files unless -a is set
    for (const auto &entry : fs::directory_iterator(dir))
    {
        string name = entry.path().filename().string();
        if (!opts.showAll && name[0] == '.')
            continue;
        entries.push_back(entry);
    }

    // --- Sorting ---
    if (opts.sortByTime)
    {
        // -t: newest modification time first
        sort(entries.begin(), entries.end(), [](const fs::directory_entry &a, const fs::directory_entry &b)
             { return fileTime(a.path()) > fileTime(b.path()); });
    }
    else if (opts.sortBySize)
    {
        // -S: largest file first; directories are treated as 0 bytes
        sort(entries.begin(), entries.end(), [](const fs::directory_entry &a, const fs::directory_entry &b)
             { return fileSize(a) > fileSize(b); });
    }
    else
    {
        // Default: case-insensitive alphabetical order
        sort(entries.begin(), entries.end(), [](const fs::directory_entry &a, const fs::directory_entry &b)
             {
              string na = a.path().filename().string();
              string nb = b.path().filename().string();
              transform(na.begin(), na.end(), na.begin(), ::tolower);
              transform(nb.begin(), nb.end(), nb.begin(), ::tolower);
              return na < nb; });
    }

    // -r: flip whichever order was just established
    if (opts.reverse)
        reverse(entries.begin(), entries.end());

    // Print every collected entry
    for (const auto &entry : entries)
        printEntry(entry, opts);

    // Short-mode entries don't self-terminate; print the trailing newline here
    if (!opts.longFormat && !opts.onePerLine)
        cout << "\n";

    // -R: after the current directory is printed, recurse into each subdir.
    // Print a "path:" header before each subdirectory's contents (same as GNU ls).
    if (opts.recursive)
    {
        for (const auto &entry : entries)
        {
            if (fs::is_directory(entry.status()))
            {
                cout << "\n"
                     << entry.path().string() << ":\n";
                listDirectory(entry.path(), opts, false);
            }
        }
    }
}

int handleLs(const ParsedInput &parsed, ShellState &state)
{
    // Build options from parsed flags
    LsOptions opts;
    opts.showAll = hasFlag(parsed, 'a', "all");
    opts.longFormat = hasFlag(parsed, 'l', "");
    opts.humanRead = hasFlag(parsed, 'h', "human-readable");
    opts.sortByTime = hasFlag(parsed, 't', "");
    opts.sortBySize = hasFlag(parsed, 'S', "");
    opts.reverse = hasFlag(parsed, 'r', "reverse");
    opts.recursive = hasFlag(parsed, 'R', "recursive");
    opts.onePerLine = hasFlag(parsed, '1', "");
    opts.dirItself = hasFlag(parsed, 'd', "directory");

    // Default target is current working directory when no paths are given
    vector<string> targets = parsed.files;
    if (targets.empty())
        targets.push_back(".");

    int exitCode = 0;
    // With multiple targets, print "target:" headers to separate each block
    bool printHeader = targets.size() > 1;

    for (const string &target : targets)
    {
        fs::path resolved = resolvePath(target);

        // Non-existent path: report error, continue to next target
        if (!fs::exists(resolved))
        {
            cerr << "ls: cannot access '" << target << "': No such file or directory\n";
            exitCode = 1;
            continue;
        }

        if (printHeader)
            cout << target << ":\n";

        if (fs::is_directory(resolved) && !opts.dirItself)
        {
            // Normal directory: list its contents
            listDirectory(resolved, opts);
        }
        else
        {
            // Either a plain file, or -d was passed for a directory:
            // treat the path itself as the single entry to display
            fs::directory_entry entry(resolved);
            printEntry(entry, opts);
            // Short mode needs its trailing newline since no listDirectory call was made
            if (!opts.longFormat && !opts.onePerLine)
                cout << "\n";
        }

        if (printHeader)
            cout << "\n";
    }

    return state.recordCommandExitCode(exitCode);
}