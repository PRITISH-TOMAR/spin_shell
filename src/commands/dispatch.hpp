#pragma once
#include "commands.hpp"
#include "../utils/parser/parser.hpp"
#include "../utils/shell_state/shell_state.hpp"

// Central dispatch: maps a resolved Command enum to its handler.
// EXIT is excluded — callers must handle it themselves (return / break loop).
// Returns the handler's exit code, or 0 for unimplemented commands.
int dispatchCommand(Command cmd, const ParsedInput& parsed, ShellState& state);
