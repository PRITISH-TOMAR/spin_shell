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
