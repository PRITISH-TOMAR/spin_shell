#include "input_handler.hpp"

static void trimLeadingAndTrailingWhitespace(string &input)
{
    size_t start = input.find_first_not_of(" \t\r\n");
    size_t end   = input.find_last_not_of(" \t\r\n");
    input = (start == string::npos) ? "" : input.substr(start, end - start + 1);
}

bool prepareInputForDispatch(string &input, ShellState &state, ParsedInput &out)
{
    trimLeadingAndTrailingWhitespace(input);

    if (input.empty()) return true;

    if (input == "$?")
    {
        state.displayLastCommandExitCode();
        return true;
    }

    state.expandShellVariablesInPlace(input);

    out = parseInput(input);

    if (state.parseAndStoreVariableAssignment(out.command))
        return true;

    return false;
}
