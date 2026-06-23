#include "mv.hpp"
#include "src/utils/path/path.hpp"
#include "src/utils/handlers/flag_set.hpp"
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

struct MvOptions {
    bool noClobber = false;
    bool verbose   = false;
};

static bool moveEntry(const fs::path& src, const fs::path& dst,
                      const string& srcStr, const MvOptions& opts)
{
    error_code ec;

    if (!fs::exists(src, ec)) {
        cerr << "mv: cannot stat '" << srcStr << "': No such file or directory\n";
        return false;
    }

    if (opts.noClobber && fs::exists(dst, ec))
        return true;

    fs::rename(src, dst, ec);
    if (ec) {
        // Cross-device move: copy then delete
        if (fs::is_directory(src, ec)) {
            fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
            if (!ec) fs::remove_all(src, ec);
        } else {
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
            if (!ec) fs::remove(src, ec);
        }
    }

    if (ec) {
        cerr << "mv: cannot move '" << srcStr << "': " << ec.message() << "\n";
        return false;
    }

    if (opts.verbose)
        cout << "'" << srcStr << "' -> '" << dst.string() << "'\n";

    return true;
}

int handleMv(const ParsedInput& parsed, ShellState& state) {
    FlagSet flags(MV_FLAGS);
    flags.parse(parsed);

    MvOptions opts;
    opts.noClobber = flags.has('n');
    opts.verbose   = flags.has('v');

    if (parsed.files.size() < 2) {
        cerr << "mv: missing file operand\n";
        cerr << "Usage: mv [OPTION]... SOURCE DEST\n";
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
        if (!moveEntry(src, target, srcStr, opts))
            exitCode = 1;
    } else {
        error_code ec;
        if (!fs::is_directory(dst, ec)) {
            cerr << "mv: target '" << dstStr << "' is not a directory\n";
            return state.recordCommandExitCode(1);
        }
        for (size_t i = 0; i + 1 < parsed.files.size(); ++i) {
            const string& srcStr = parsed.files[i];
            fs::path src = resolvePath(srcStr);
            if (!moveEntry(src, dst / src.filename(), srcStr, opts))
                exitCode = 1;
        }
    }

    return state.recordCommandExitCode(exitCode);
}
