#include "mkdir.hpp"
#include "src/utils/path/path.hpp"
#include "src/utils/handlers/flag_set.hpp"
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = filesystem;

struct MkdirOptions
{
    bool parents = false;
    bool verbose = false;
    string mode;
};

// Converts "755" → fs::perms via stoi with base 8.
static bool applyMode(const fs::path &dir, const string &modeStr)
{
    try
    {
        int octal = stoi(modeStr, nullptr, 8);
        fs::permissions(dir, static_cast<fs::perms>(octal));
        return true;
    }
    catch (...)
    {
        cerr << "mkdir: invalid mode: '" << modeStr << "'\n";
        return false;
    }
}

static bool createDir(const fs::path &path, const string &original, const MkdirOptions &opts)
{
    error_code ec;

    if (opts.parents)
    {
        // create_directories creates all missing parents
        fs::create_directories(path, ec);
    }
    else
    {
        // create_directory errors if parent missing or dir already exists
        if (fs::exists(path))
        {
            cerr << "mkdir: cannot create directory '" << original << "': File exists\n";
            return false;
        }
        fs::create_directory(path, ec);
    }

    if (ec)
    {
        cerr << "mkdir: cannot create directory '" << original << "': " << ec.message() << "\n";
        return false;
    }

    if (opts.verbose)
        cout << "mkdir: created directory '" << original << "'\n";

    return true;
}

int handleMkdir(const ParsedInput &parsed, ShellState &state)
{
    if (parsed.files.empty())
    {
        cerr << "mkdir: missing operand\n";
        cerr << "Usage: mkdir [OPTION]... DIRECTORY...\n";
        return state.recordCommandExitCode(1);
    }

    FlagSet flags(MKDIR_FLAGS);
    flags.parse(parsed);

    MkdirOptions opts;
    opts.parents = flags.has('p');
    opts.verbose = flags.has('v');
    opts.mode    = flags.value('m');

    int exitCode = 0;

    for (const string &target : parsed.files)
    {
        if (flags.isValueToken(target))
            continue;

        fs::path resolved = resolvePath(target);

        if (!createDir(resolved, target, opts))
        {
            exitCode = 1;
            continue;
        }

        // Apply -m permissions after creation
        if (!opts.mode.empty())
        {
            if (!applyMode(resolved, opts.mode))
                exitCode = 1;
        }
    }

    return state.recordCommandExitCode(exitCode);
}
