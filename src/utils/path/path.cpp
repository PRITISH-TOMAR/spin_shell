#include <cstdlib>
#include "path.hpp"

fs::path resolvePath(const string &input)
{
    if (input.empty())
        return fs::current_path();

    if (input[0] == '~')
    {
        const char *home = getenv("HOME"); // macOS/Linux
        if (!home)
            home = getenv("USERPROFILE"); // Windows

        if (home)
        {
            return fs::path(home) / input.substr(1);
        }
    }

    fs::path p(input);

    if (p.is_absolute())
    {
        return p;
    }
    return fs::current_path() / p;
}