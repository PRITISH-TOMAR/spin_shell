# codecrafters-shell-cpp

A POSIX-style shell built from scratch in C++23.

## Features

- Built-in commands: `cd`, `pwd`, `exit`, `echo`, `cat`, `ls`, `grep`, `mkdir`, `rm`, `cp`, `mv`, `touch`, `clear`, `history`, `help`
- External command execution via `$PATH`
- IO redirection: `>`, `>>`, `<`, `2>`, `2>&1`
- Pipes: `cmd1 | cmd2 | ...`
- Variable expansion: `$VAR`, `$?`
- Glob expansion: `*`, `?`
- Shell variable assignment: `VAR=value`

---

## Installation

### Prerequisites

| Tool | Version |
|---|---|
| CMake | 3.20+ |
| MinGW-w64 (Windows) or GCC (Linux/macOS) | C++23-capable |
| Git | any |

### Windows (MinGW)

```bash
# 1. Clone the repository
git clone https://github.com/<your-username>/codecrafters-shell-cpp.git
cd codecrafters-shell-cpp

# 2. Configure
cmake -G "MinGW Makefiles" -B build -S .

# 3. Build
cmake --build build

# 4. Run
./build/shell
```

Or use the convenience script (cleans build/ each time):

```bash
bash run.sh
```

### Linux / macOS (GCC/Clang)

```bash
git clone https://github.com/<your-username>/codecrafters-shell-cpp.git
cd codecrafters-shell-cpp

cmake -B build -S .
cmake --build build

./build/shell
```

> On macOS you may need to install CMake via `brew install cmake` and ensure Xcode CLT is present (`xcode-select --install`).

### Optional: add to PATH

To use `shell` from anywhere:

```bash
# Linux / macOS
sudo cp build/shell /usr/local/bin/myshell

# Windows — add build/ to your system PATH, or copy the binary:
copy build\shell.exe C:\Windows\System32\myshell.exe
```

---

## Project Structure

```
src/
  main.cpp                        # REPL loop
  commands/
    commands.hpp/.cpp             # Command enum + maps
    dispatch.cpp                  # Central switch dispatcher
    handlers/
      builtins/                   # cd, pwd, exit, help, history
      tools/                      # cat, echo, ls, grep, mkdir, ...
  utils/
    parser/                       # parseInput()
    shell_state/                  # ShellState struct
    path/                         # resolvePath()
    redirections/                 # Redirection types + RedirGuard
    executors/                    # executeExternalCommand()
```

---

## Development

```bash
# Build and run in one step
bash run.sh

# Run tests (once CI is set up)
# See .github/workflows/build.yml
```

See [ROADMAP.md](ROADMAP.md) for planned features and next steps.
