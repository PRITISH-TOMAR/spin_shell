#include "cp.hpp"
#include "src/utils/path/path.hpp"
#include "src/utils/handlers/flag_set.hpp"
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

struct CpOptions {
    bool recursive = false;
    bool noClobber = false;
    bool verbose   = false;
};

static bool copyEntry(const fs::path& src, const fs::path& dst,
                      const string& srcStr, const CpOptions& opts)
{
    error_code ec;

    if (!fs::exists(src, ec)) {
        cerr << "cp: cannot stat '" << srcStr << "': No such file or directory\n";
        return false;
    }

    if (fs::is_directory(src, ec)) {
        if (!opts.recursive) {
            cerr << "cp: -r not specified; omitting directory '" << srcStr << "'\n";
            return false;
        }
        if (opts.noClobber && fs::exists(dst, ec))
            return true;
        fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    } else {
        if (opts.noClobber && fs::exists(dst, ec))
            return true;
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    }

    if (ec) {
        cerr << "cp: cannot copy '" << srcStr << "': " << ec.message() << "\n";
        return false;
    }

    if (opts.verbose)
        cout << "'" << srcStr << "' -> '" << dst.string() << "'\n";

    return true;
}

int handleCp(const ParsedInput& parsed, ShellState& state) {
    FlagSet flags(CP_FLAGS);
    flags.parse(parsed);

    CpOptions opts;
    opts.recursive = flags.has('r') || flags.has('R');
    opts.noClobber = flags.has('n');
    opts.verbose   = flags.has('v');

    if (parsed.files.size() < 2) {
        cerr << "cp: missing file operand\n";
        cerr << "Usage: cp [OPTION]... SOURCE DEST\n";
        return state.recordCommandExitCode(1);
    }

    const string& dstStr = parsed.files.back();
    fs::path dst = resolvePath(dstStr);
    int exitCode = 0;

    if (parsed.files.size() == 2) {
        const string& srcStr = parsed.files[0];
        fs::path src = resolvePath(srcStr);
        error_code ec;
        fs::path target = fs::is_directory(dst, ec) ? dst / src.filename() : dst;
        if (!copyEntry(src, target, srcStr, opts))
            exitCode = 1;
    } else {
        error_code ec;
        if (!fs::is_directory(dst, ec)) {
            cerr << "cp: target '" << dstStr << "' is not a directory\n";
            return state.recordCommandExitCode(1);
        }
        for (size_t i = 0; i + 1 < parsed.files.size(); ++i) {
            const string& srcStr = parsed.files[i];
            fs::path src = resolvePath(srcStr);
            if (!copyEntry(src, dst / src.filename(), srcStr, opts))
                exitCode = 1;
        }
    }

    return state.recordCommandExitCode(exitCode);
}
