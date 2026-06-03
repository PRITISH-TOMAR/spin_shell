#include "rm.hpp"
#include "src/utils/path/path.hpp"
#include "src/utils/handlers/flag_set.hpp"
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = filesystem;

struct RmOptions
{
    bool recursive = false;
    bool force = false;
    bool interactive = false;
    bool verbose = false;
};

static bool confirmRemoval(const string &path)
{
    cerr << "rm: remove '" << path << "'? ";
    string answer;
    getline(cin, answer);
    return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
}

static bool removeEntry(const fs::path &path, const string &original, const RmOptions &opts)
{
    error_code ec;

    if (!fs::exists(path, ec))
    {
        if (!opts.force)
            cerr << "rm: cannot remove '" << original << "': No such file or directory\n";
        return opts.force;
    }

    if (fs::is_directory(path, ec))
    {
        if (!opts.recursive)
        {
            cerr << "rm: cannot remove '" << original << "': Is a directory\n";
            return false;
        }
    }

    if (opts.interactive && !confirmRemoval(original))
        return true; // skipping is not an error

    if (opts.recursive && fs::is_directory(path, ec))
        fs::remove_all(path, ec);
    else
        fs::remove(path, ec);

    if (ec)
    {
        cerr << "rm: cannot remove '" << original << "': " << ec.message() << "\n";
        return false;
    }

    if (opts.verbose)
        cout << "removed '" << original << "'\n";

    return true;
}

int handleRm(const ParsedInput &parsed, ShellState &state)
{
    FlagSet flags(RM_FLAGS);
    flags.parse(parsed);

    RmOptions opts;
    opts.recursive = flags.has('r');
    opts.force = flags.has('f');
    opts.interactive = flags.has('i');
    opts.verbose = flags.has('v');

    if (parsed.files.empty())
    {
        if (!opts.force)
        {
            cerr << "rm: missing operand\n";
            cerr << "Usage: rm [OPTION]... FILE...\n";
            return state.recordCommandExitCode(1);
        }
        return state.recordCommandExitCode(0);
    }

    int exitCode = 0;

    for (const string &target : parsed.files)
    {
        fs::path resolved = resolvePath(target);
        if (!removeEntry(resolved, target, opts))
            exitCode = 1;
    }

    return state.recordCommandExitCode(exitCode);
}
