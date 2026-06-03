#include "touch.hpp"
#include "src/utils/path/path.hpp"
#include "src/utils/handlers/flag_set.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>

namespace fs = filesystem;
using fs_time = fs::file_time_type;

struct TouchOptions {
    bool accessOnly = false;
    bool modifyOnly = false;
    bool noCreate   = false;
    string dateStr;
    string stampStr;
    string refFile;
};

// Parses a -d datetime string ("2024-01-01 12:00" or "2024-01-01T12:00")
static time_t parseDateString(const string& s) {
    tm t = {};

    istringstream ss(s);
    ss >> get_time(&t, "%Y-%m-%d %H:%M");
    if (ss.fail()) {
        istringstream ss2(s);
        ss2 >> get_time(&t, "%Y-%m-%dT%H:%M");
        if (ss2.fail()) return -1;
    }
    t.tm_isdst = -1;
    return mktime(&t);
}

// Parses a -t timestamp in [[CC]YY]MMDDhhmm[.ss] format into a time_t.
static time_t parseStampString(const string& s) {
    // Strip optional seconds (.ss) at the end
    string base = s;
    int secs = 0;
    size_t dot = s.find('.');
    if (dot != string::npos) {
        secs = stoi(s.substr(dot + 1));
        base = s.substr(0, dot);
    }

    // base must be 8 (MMDDhhmm), 10 (YYMMDDhhmm), or 12 (CCYYMMDDhhmm) chars
    if (base.size() != 8 && base.size() != 10 && base.size() != 12) return -1;

    tm t = {};
    t.tm_isdst = -1;
    t.tm_sec = secs;

    try {
        if (base.size() == 12) {
            t.tm_year = stoi(base.substr(0, 4)) - 1900;
            base = base.substr(4);
        } else if (base.size() == 10) {
            int yy = stoi(base.substr(0, 2));
            // 2-digit year: 00-68 -> 2000s, 69-99 -> 1900s
            t.tm_year = yy >= 69 ? 1900 + yy - 1900 : 2000 + yy - 1900;
            base = base.substr(2);
        } else {
            // No year given - use current year
            time_t now = time(nullptr);
            t.tm_year = localtime(&now)->tm_year;
        }
        t.tm_mon  = stoi(base.substr(0, 2)) - 1;
        t.tm_mday = stoi(base.substr(2, 2));
        t.tm_hour = stoi(base.substr(4, 2));
        t.tm_min  = stoi(base.substr(6, 2));
    } catch (...) {
        return -1;
    }

    return mktime(&t);
}

// Converts a time_t to fs::file_time_type so it can be passed to
// fs::last_write_time(). Bridges system_clock -> file_time_type.
static fs_time toFileTime(time_t t) {
    auto sysPoint = chrono::system_clock::from_time_t(t);
    auto duration = sysPoint.time_since_epoch();
    return fs_time(chrono::duration_cast<fs_time::duration>(duration));
}

// Creates an empty file at the given path.
// Simply opening with ofstream in default mode creates it if missing.
static bool createFile(const fs::path& path, const string& original) {
    ofstream f(path, ios::app);
    if (!f) {
        cerr << "touch: cannot create '" << original << "': Permission denied\n";
        return false;
    }
    return true;
}

// Updates the modification time of path to newTime.
static bool applyTime(const fs::path& path, const string& original, fs_time newTime) {
    error_code ec;
    fs::last_write_time(path, newTime, ec);
    if (ec) {
        cerr << "touch: cannot set time on '" << original << "': " << ec.message() << "\n";
        return false;
    }
    return true;
}

int handleTouch(const ParsedInput& parsed, ShellState& state) {
    if (parsed.files.empty() && parsed.rawArgs.empty()) {
        cerr << "touch: missing file operand\n";
        cerr << "Usage: touch [OPTION]... FILE...\n";
        return state.recordCommandExitCode(1);
    }

    FlagSet flags(TOUCH_FLAGS);
    flags.parse(parsed);

    TouchOptions opts;
    opts.accessOnly = flags.has('a');
    opts.modifyOnly = flags.has('m');
    opts.noCreate   = flags.has('c');
    opts.dateStr    = flags.value('d');
    opts.stampStr   = flags.value('t');
    opts.refFile    = flags.value('r');

    // Resolve the target time to apply.
    // Priority: -r > -d > -t > now
    fs_time targetTime;
    bool hasCustomTime = false;

    if (!opts.refFile.empty()) {
        // -r: borrow timestamps from reference file
        fs::path refPath = resolvePath(opts.refFile);
        if (!fs::exists(refPath)) {
            cerr << "touch: failed to get attributes of '" << opts.refFile << "': No such file or directory\n";
            return state.recordCommandExitCode(1);
        }
        error_code ec;
        targetTime = fs::last_write_time(refPath, ec);
        if (ec) {
            cerr << "touch: cannot read time from '" << opts.refFile << "': " << ec.message() << "\n";
            return state.recordCommandExitCode(1);
        }
        hasCustomTime = true;

    } else if (!opts.dateStr.empty()) {
        // -d: parse human-readable datetime string
        time_t t = parseDateString(opts.dateStr);
        if (t == -1) {
            cerr << "touch: invalid date format: '" << opts.dateStr << "'\n";
            return state.recordCommandExitCode(1);
        }
        targetTime = toFileTime(t);
        hasCustomTime = true;

    } else if (!opts.stampStr.empty()) {
        // -t: parse [[CC]YY]MMDDhhmm[.ss] timestamp
        time_t t = parseStampString(opts.stampStr);
        if (t == -1) {
            cerr << "touch: invalid timestamp: '" << opts.stampStr << "'\n";
            return state.recordCommandExitCode(1);
        }
        targetTime = toFileTime(t);
        hasCustomTime = true;
    }

    // If no custom time, use current time
    if (!hasCustomTime)
        targetTime = fs_time::clock::now();

    int exitCode = 0;

    for (const string& target : parsed.files) {
        if (flags.isValueToken(target)) continue;

        fs::path resolved = resolvePath(target);

        if (!fs::exists(resolved)) {
            // -c: do not create, silently skip
            if (opts.noCreate) continue;

            if (!createFile(resolved, target)) {
                exitCode = 1;
                continue;
            }
        }

        // -a only -> skip modifying write time (access time not portable via fs::)
        // -m only or default -> update modification time
        if (!opts.accessOnly) {
            if (!applyTime(resolved, target, targetTime))
                exitCode = 1;
        }
    }

    return state.recordCommandExitCode(exitCode);
}
