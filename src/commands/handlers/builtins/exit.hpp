#pragma once
#include "src/utils/parser/parser.hpp"
#include "src/utils/shell_state/shell_state.hpp"

[[noreturn]] void handleExit(const ParsedInput& parsed, ShellState& state);
