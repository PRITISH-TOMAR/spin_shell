#include "path_glob.hpp"
#include "glob_matcher.hpp"
#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;
using std::error_code;

// Splits a pattern like "src/**/*.cpp" into ["src", "**", "*.cpp"].
static vector<string> splitSegments(const string &pattern)
{
    vector<string> segs;
    string seg;
    for (char c : pattern)
    {
        if (c == '/')
        {
            if (!seg.empty())
                segs.push_back(seg);
            seg.clear();
        }
        else
        {
            seg += c;
        }
    }
    if (!seg.empty())
        segs.push_back(seg);
    return segs;
}

// Recursive walker.
// segs:      all pattern segments
// segIdx:    which segment we're currently matching
// fsDir:     filesystem directory to iterate
// relPrefix: relative path built so far (e.g. "src/"), prepended to results
// results:   accumulates matched paths
static void walk(
    const vector<string> &segs,
    size_t segIdx,
    const fs::path &fsDir,
    const string &relPrefix,
    vector<string> &results)
{
    if (segIdx >= segs.size())
        return;

    const string &seg = segs[segIdx];
    bool isLast = (segIdx == segs.size() - 1);

    if (seg == "**")
    {
        if (isLast)
        {
            // "**" as last segment: collect every entry at any depth
            error_code ec;
            for (auto &entry : fs::directory_iterator(fsDir, ec))
            {
                string name = entry.path().filename().string();
                if (!name.empty() && name[0] == '.')
                    continue;
                results.push_back(relPrefix + name);
                if (entry.is_directory())
                    walk(segs, segIdx, entry.path(), relPrefix + name + "/", results);
            }
            return;
        }

        // "**" matches zero directory levels: try next segment in current dir
        walk(segs, segIdx + 1, fsDir, relPrefix, results);

        // "**" matches one or more levels: descend into every subdir
        error_code ec;
        for (auto &entry : fs::directory_iterator(fsDir, ec))
        {
            if (!entry.is_directory())
                continue;
            string name = entry.path().filename().string();
            if (!name.empty() && name[0] == '.')
                continue;
            walk(segs, segIdx, entry.path(), relPrefix + name + "/", results);
        }
        return;
    }

    // Normal segment (may contain *, ?, [...])
    error_code ec;
    for (auto &entry : fs::directory_iterator(fsDir, ec))
    {
        string name = entry.path().filename().string();

        // Dot-file rule: skip hidden entries unless pattern starts with '.'
        if (!name.empty() && name[0] == '.' && (seg.empty() || seg[0] != '.'))
            continue;

        if (!matchGlob(seg, name))
            continue;

        if (isLast)
        {
            results.push_back(relPrefix + name);
        }
        else if (entry.is_directory())
        {
            walk(segs, segIdx + 1, entry.path(), relPrefix + name + "/", results);
        }
    }
}

vector<string> pathGlob(const string &pattern, const string &cwd)
{
    vector<string> segs = splitSegments(pattern);
    if (segs.empty())
        return {};

    vector<string> results;
    walk(segs, 0, fs::path(cwd), "", results);
    sort(results.begin(), results.end());
    return results;
}
