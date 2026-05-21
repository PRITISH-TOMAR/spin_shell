#pragma once
#include <string>
#include "../parser/parser.hpp"
#include "../shell_state/shell_state.hpp"

using namespace std;

// returns true = skip to next iteration, false = proceed to dispatch
bool preprocessInput(string &input, ShellState &state, ParsedInput &out);
