#pragma once
#include "src/utils/shell_state/shell_state.hpp"
#include <vector>
#include <string>

using namespace std;


// executes the pipelne, CMD1 output -> CMD2 Input -> ...
// final output writes to real stdout, errors to real stderr
void runPipeline(const vector<string>& segments, ShellState& shellState);
