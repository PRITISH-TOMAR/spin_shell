#include "input_handler.hpp"
#include "glob/brace_expander.hpp"
#include "glob/path_glob.hpp"
#include <algorithm>

static bool hasGlobChars(const string &s)
{
    for (char c : s)
        if (c == '*' || c == '?' || c == '[')
            return true;
    return false;
}

// Expands unquoted glob/brace tokens in parsed.rawArgs in-place.
// Pipeline per token: expandBraces → pathGlob (only if pattern has glob chars).
// Brace-only results (no * ? [) are kept as-is without filesystem check.
// Glob patterns with no filesystem match keep the literal pattern (nullglob-off).
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

        // Step 1: brace expansion (purely textual)
        vector<string> braceExpanded = expandBraces(token);

        // Step 2: for each brace-expanded pattern, glob-expand if it has wildcards
        vector<string> finalTokens;
        for (const string &pattern : braceExpanded)
        {
            if (hasGlobChars(pattern))
            {
                vector<string> m = pathGlob(pattern, cwd);
                if (m.empty())
                    finalTokens.push_back(pattern); // no match → keep literal pattern
                else
                    finalTokens.insert(finalTokens.end(), m.begin(), m.end());
            }
            else
            {
                finalTokens.push_back(pattern); // no wildcards → keep as-is
            }
        }

        sort(finalTokens.begin(), finalTokens.end());

        for (const string &t : finalTokens)
        {
            newRawArgs.push_back(t);
            if (!isFlag)
                newFiles.push_back(t);
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
    parseRedirections(input, redirections);

    out = parseInput(input);
    out.redirections = move(redirections);
    expandGlobs(out, state.currentDirectory);

    if (state.parseAndStoreVariableAssignment(out.command))
        return true;

    return false;
}
