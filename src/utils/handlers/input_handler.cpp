#include "input_handler.hpp"

bool preprocessInput(string &input, ShellState &state, ParsedInput &out)
{
    if (input == "$?")
    {
        state.printExitStatus();
        return true;
    }

    state.expandInput(input);

    out = parseInput(input);

    if (state.tryAssign(out.command))
        return true;

    return false;
}
