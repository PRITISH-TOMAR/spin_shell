#pragma once
#include <string>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

// Resolves any path to absolute:
//   relative  → cwd / path
//   absolute  → as is
//   ~         → home / path
fs::path resolvePath(const string& input);