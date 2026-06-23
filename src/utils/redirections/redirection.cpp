#include <redirection.hpp>
#include <src/utils/quote_utils.hpp>

#include <cctype>
#include <cstring>
#include <array>

struct OpEntry
{
    const char *op;
    RedirectionType type;
    bool hasFile; // true if the operator is followed by a file name
};

static const array<OpEntry, 6> &redirectionOps()
{
    static const array<OpEntry, 6> ops = {{
        {"2>&1", RedirectionType::STDERR_TO_STDOUT, false},
        {"&>", RedirectionType::BOTH_FILE, true},
        {"2>", RedirectionType::STDERR_FILE, true},
        {">>", RedirectionType::STDOUT_APPEND, true},
        {">", RedirectionType::STDOUT_OVERWRITE, true},
        {"<", RedirectionType::STDIN_FILE, true},
    }};
    return ops;
}

// Scans `raw` for redirection operators, strips them (and their target filenames)
// from the string, and records each one in `out`.

//   raw = "echo hello > out.txt 2> err.txt"
//   after call: raw = "echo hello", out = [{ STDOUT_OVERWRITE, "out.txt" }, { STDER_FILE, "err.txt" }]

// One redirection is removed per iteration, then the loop restarts from the beginning because erasing from `raw` invalidates all positions.

void parseRedirections(string &raw, vector<Redirection> &out)
{
    static const OpEntry ops[] = {
        {"2>&1", RedirectionType::STDERR_TO_STDOUT, false},
        {"&>", RedirectionType::BOTH_FILE, true},
        {"2>", RedirectionType::STDERR_FILE, true},
        {">>", RedirectionType::STDOUT_APPEND, true},
        {">", RedirectionType::STDOUT_OVERWRITE, true},
        {"<", RedirectionType::STDIN_FILE, true},
    };

    string result;
    result.reserve(raw.size());
    size_t i = 0;

    while (i < raw.size())
    {
        bool matched = false;

        if (!isInQuotes(raw, i))
        {
            for (const auto &entry : ops)
            {
                size_t opLen = strlen(entry.op);

                if (i + opLen > raw.size())
                    continue;
                if (raw.compare(i, opLen, entry.op) != 0)
                    continue;

                size_t pos = i + opLen;
                string file;

                if (entry.hasFile)
                {
                    while (pos < raw.size() && isspace((unsigned char)raw[pos]))
                        ++pos;

                    size_t fe = pos;
                    while (fe < raw.size() && !isspace((unsigned char)raw[fe]))
                        ++fe;

                    file = raw.substr(pos, fe - pos);
                    pos = fe;
                }

                while (!result.empty() && result.back() == ' ')
                    result.pop_back();

                out.push_back({entry.type, file});
                i = pos;
                matched = true;
                break;
            }
        }

        if (!matched)
        {
            result += raw[i];
            ++i;
        }
    }

    raw = move(result);
}