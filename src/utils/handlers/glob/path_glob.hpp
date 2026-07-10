#pragma once
#include <string>
#include <vector>
using std::string;
using std::vector;

// Expands a single glob pattern (no braces) against the filesystem.
// cwd: base directory for relative patterns.
// Returns sorted matching paths relative to cwd. Empty vector if no match.
vector<string> pathGlob(const string &pattern, const string &cwd);
