# spin_shell

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

## Download Pre-built Binary

Pre-built binaries are available on the [Releases page](https://github.com/PRITISH-TOMAR/spin_shell/releases/latest).

| Platform | File |
|---|---|
| Windows (x64) | `shell-windows-x64.exe` |
| Linux (x64) | `shell-linux-x64` |
| macOS (Apple Silicon) | `shell-macos-arm64` |

A `checksums.txt` (SHA-256) is included with every release to verify integrity.

### Windows

```powershell
# 1. Download shell-windows-x64.exe from the Releases page

# 2. (Optional) Verify checksum — open PowerShell in the download folder
Get-FileHash shell-windows-x64.exe -Algorithm SHA256

# 3a. Run directly
.\shell-windows-x64.exe

# 3b. Or install system-wide: copy to a folder that is on your PATH
#     Example using the user's local bin (no admin needed):
mkdir "$env:USERPROFILE\bin" -Force
copy shell-windows-x64.exe "$env:USERPROFILE\bin\shell.exe"
# Then add %USERPROFILE%\bin to your PATH via System Properties → Environment Variables
```

### Add to Windows Terminal

Run in **PowerShell**:

```powershell
iwr https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/shell-windows-x64.exe -OutFile "$env:USERPROFILE\shell.exe"
$f = "$env:LOCALAPPDATA\Packages\Microsoft.WindowsTerminal_8wekyb3d8bbwe\LocalState\settings.json"
$c = Get-Content $f -Raw | ConvertFrom-Json
$c.profiles.list += [pscustomobject]@{ name="spin_shell"; guid="{a8f1c2d3-4e5b-6f7a-8b9c-0d1e2f3a4b5c}"; commandline="$env:USERPROFILE\shell.exe"; startingDirectory="%USERPROFILE%" }
$c | ConvertTo-Json -Depth 10 | Set-Content $f
```

Reopen Windows Terminal — **spin_shell** appears in the dropdown.

---

### Linux

```bash
# 1. Download
curl -Lo shell https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/shell-linux-x64

# 2. (Optional) Verify checksum
curl -Lo checksums.txt https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/checksums.txt
sha256sum -c checksums.txt --ignore-missing

# 3. Make executable and install
chmod +x shell
sudo mv shell /usr/local/bin/shell
```

### macOS

```bash
# 1. Download
curl -Lo shell https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/shell-macos-arm64

# 2. (Optional) Verify checksum
curl -Lo checksums.txt https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/checksums.txt
shasum -a 256 -c checksums.txt --ignore-missing

# 3. Make executable and install
chmod +x shell
sudo mv shell /usr/local/bin/shell

# 4. If macOS Gatekeeper blocks it (unsigned binary):
xattr -d com.apple.quarantine /usr/local/bin/shell
```

---

## Installation (Build from Source)

### Prerequisites

| Tool | Version |
|---|---|
| CMake | 3.20+ |
| MinGW-w64 (Windows) or GCC (Linux/macOS) | C++23-capable |
| Git | any |

### Windows (MinGW)

```bash
# 1. Clone the repository
git clone https://github.com/PRITISH-TOMAR/spin_shell.git
cd spin_shell

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
git clone https://github.com/PRITISH-TOMAR/spin_shell.git
cd spin_shell

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
