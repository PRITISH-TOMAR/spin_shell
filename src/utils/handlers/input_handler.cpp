#include "input_handler.hpp"
#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;

// Returns true if `name` matches shell glob `pat` (* = any sequence, ? = one char).
static bool matchGlob(const string &pat, const string &name)
{
    size_t pi = 0, ni = 0, starPi = string::npos, starNi = 0;
    while (ni < name.size())
    {
        if (pi < pat.size() && (pat[pi] == name[ni] || pat[pi] == '?'))
        {
            ++pi;
            ++ni;
        }
        else if (pi < pat.size() && pat[pi] == '*')
        {
            starPi = pi++;
            starNi = ni;
        }
        else if (starPi != string::npos)
        {
            pi = starPi + 1;
            ni = ++starNi;
        }
        else
        {
            return false;
        }
    }
    while (pi < pat.size() && pat[pi] == '*')
        ++pi;
    return pi == pat.size();
}

// Expands unquoted glob tokens in parsed.rawArgs in-place.
// Tokens that had unquoted * ? [ are replaced with sorted filesystem matches.
// If no matches, the literal token is kept (bash nullglob-off behaviour).
static void expandGlobs(ParsedInput &parsed, const string &cwd)
{
    vector<string> newRawArgs;
    vector<string> newFiles;

    for (size_t i = 0; i < parsed.rawArgs.size(); ++i)
    {
        const string &token = parsed.rawArgs[i];
        bool isFlag = token.size() > 1 && token[0] == '-';

        if (!parsed.rawArgsHasGlob[i])
        {
            newRawArgs.push_back(token);
            if (!isFlag)
                newFiles.push_back(token);
            continue;
        }

        vector<string> matches;
        error_code ec;
        for (auto &entry : fs::directory_iterator(cwd, ec))
        {
            string name = entry.path().filename().string();
            if (!name.empty() && name[0] == '.')
                continue; // skip hidden files
            if (matchGlob(token, name))
                matches.push_back(name);
        }
        sort(matches.begin(), matches.end());

        if (matches.empty())
        {
            newRawArgs.push_back(token);
            if (!isFlag)
                newFiles.push_back(token);
        }
        else
        {
            for (const string &m : matches)
            {
                newRawArgs.push_back(m);
                newFiles.push_back(m);
            }
        }
    }

    parsed.rawArgs = move(newRawArgs);
    parsed.files = move(newFiles);
    parsed.rawArgsHasGlob.clear();
}

static void trimLeadingAndTrailingWhitespace(string &input)
{
    size_t start = input.find_first_not_of(" \t\r\n");
    size_t end = input.find_last_not_of(" \t\r\n");
    input = (start == string::npos) ? "" : input.substr(start, end - start + 1);
}

bool prepareInputForDispatch(string &input, ShellState &state, ParsedInput &out)
{
    trimLeadingAndTrailingWhitespace(input);

    if (input.empty())
        return true;

    if (input == "$?")
    {
        state.displayLastCommandExitCode();
        return true;
    }

    state.expandShellVariablesInPlace(input);

    vector<Redirection> redirections;
    parseRedirections(input, out.redirections);

    out = parseInput(input);
    out.redirections = move(redirections);
    expandGlobs(out, state.currentDirectory);

    if (state.parseAndStoreVariableAssignment(out.command))
        return true;

    return false;
}
