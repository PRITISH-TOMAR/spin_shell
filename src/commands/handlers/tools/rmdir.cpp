#include "rmdir.hpp"
#include "src/utils/path/path.hpp"
#include "src/utils/handlers/flag_set.hpp"
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = filesystem;

static bool removeSingleDir(const fs::path& path, const string& display, bool verbose)
{
    error_code ec;

    if (!fs::exists(path, ec))
    {
        cerr << "rmdir: failed to remove '" << display << "': No such file or directory\n";
        return false;
    }

    if (!fs::is_directory(path, ec))
    {
        cerr << "rmdir: failed to remove '" << display << "': Not a directory\n";
        return false;
    }

    if (!fs::is_empty(path, ec))
    {
        cerr << "rmdir: failed to remove '" << display << "': Directory not empty\n";
        return false;
    }

    fs::remove(path, ec);
    if (ec)
    {
        cerr << "rmdir: failed to remove '" << display << "': " << ec.message() << "\n";
        return false;
    }

    if (verbose)
        cout << "rmdir: removing directory, '" << display << "'\n";

    return true;
}

int handleRmdir(const ParsedInput& parsed, ShellState& state)
{
    FlagSet flags(RMDIR_FLAGS);
    flags.parse(parsed);

    bool parents = flags.has('p');
    bool verbose = flags.has('v');

    if (parsed.files.empty())
    {
        cerr << "rmdir: missing operand\n";
        cerr << "Usage: rmdir [-pv] DIRECTORY...\n";
        return state.recordCommandExitCode(1);
    }

    int exitCode = 0;

    for (const string& target : parsed.files)
    {
        fs::path resolved = resolvePath(target);

        if (!parents)
        {
            if (!removeSingleDir(resolved, target, verbose))
                exitCode = 1;
            continue;
        }

        // -p: remove path components from deepest to shallowest
        fs::path current = resolved;
        bool anyFailed = false;

        while (!current.empty() && current != current.parent_path())
        {
            if (!removeSingleDir(current, current.string(), verbose))
            {
                anyFailed = true;
                break;
            }
            current = current.parent_path();
        }

        if (anyFailed)
            exitCode = 1;
    }

    return state.recordCommandExitCode(exitCode);
}
