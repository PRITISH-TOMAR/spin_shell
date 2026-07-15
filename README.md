# spin_shell

A POSIX-style shell written from the ground up in C++23 — currently in active early development, with a phased roadmap toward full scripting, job control, and interactive UX.

## Table of Contents

- [Features](#features)
- [Download Pre-built Binary](#download-pre-built-binary)
- [Installation (Build from Source)](#installation-build-from-source)
- [Project Structure](#project-structure)
- [Development](#development)
- [Roadmap](#roadmap--toward-a-full-posix-compliant-shell)
- [Contributing](#contributing)

---

## Features

**Phase 1 (current):**

| Category | Support |
|---|---|
| Built-in commands | `cd`, `pwd`, `exit`, `echo`, `cat`, `ls`, `grep`, `mkdir`, `rm`, `cp`, `mv`, `touch`, `clear`, `history`, `help` |
| External execution | Runs any executable resolvable via `$PATH` |
| IO redirection | `>`, `>>`, `<`, `2>`, `2>&1` |
| Pipes | `cmd1 \| cmd2 \| ...` |
| Variable expansion | `$VAR`, `$?` |
| Glob expansion | `*`, `?` |
| Variable assignment | `VAR=value` |

The binary is called **`spin_shell`** everywhere in this doc and in every install step below. Keep that name consistent in your own setup too — so `which spin_shell` always means something.

> Looking for what's next? See the [Roadmap](#roadmap--toward-a-full-posix-compliant-shell) for Phase 2 and Phase 3.

---

## Download Pre-built Binary

Pre-built binaries are available on the [Releases page](https://github.com/PRITISH-TOMAR/spin_shell/releases/latest).

| Platform | File |
|---|---|
| Windows (x64) | `spin_shell-windows-x64.exe` |
| Linux (x64) | `spin_shell-linux-x64` |
| macOS (Apple Silicon) | `spin_shell-macos-arm64` |

Every release ships with a `checksums.txt` (SHA-256). Verify it before running anything you've downloaded from the internet.

<details>
<summary><strong>Windows</strong></summary>

```powershell
# 1. Download spin_shell-windows-x64.exe from the Releases page

# 2. Verify checksum (do this — it's an unsigned binary)
Get-FileHash spin_shell-windows-x64.exe -Algorithm SHA256
# Compare the output against the matching line in checksums.txt

# 3a. Run directly
.\spin_shell-windows-x64.exe

# 3b. Or install for your user (no admin needed):
mkdir "$env:USERPROFILE\bin" -Force
copy spin_shell-windows-x64.exe "$env:USERPROFILE\bin\spin_shell.exe"
# Then add %USERPROFILE%\bin to PATH via System Properties → Environment Variables
```

**Add to Windows Terminal:**

Windows Terminal auto-detects `spin_shell.exe` once it's on your PATH, the next time it enumerates profiles. To add it manually instead:

1. Open Windows Terminal → Settings → **Add a new profile** → New empty profile
2. Set **Command line** to the full path of `spin_shell.exe`
3. Set **Name** to `spin_shell`
4. Save

</details>

<details>
<summary><strong>Linux</strong></summary>

```bash
# 1. Download
curl -Lo spin_shell https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/spin_shell-linux-x64

# 2. Verify checksum
curl -Lo checksums.txt https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/checksums.txt
sha256sum -c checksums.txt --ignore-missing

# 3. Make executable and install
chmod +x spin_shell
sudo mv spin_shell /usr/local/bin/spin_shell
```

</details>

<details>
<summary><strong>macOS</strong></summary>

```bash
# 1. Download
curl -Lo spin_shell https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/spin_shell-macos-arm64

# 2. Verify checksum
curl -Lo checksums.txt https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/checksums.txt
shasum -a 256 -c checksums.txt --ignore-missing

# 3. Make executable and install
chmod +x spin_shell
sudo mv spin_shell /usr/local/bin/spin_shell

# 4. If macOS Gatekeeper blocks it (unsigned binary), and you've verified
#    the checksum above and trust the source, clear the quarantine flag:
xattr -d com.apple.quarantine /usr/local/bin/spin_shell
```

</details>

---

## Installation (Build from Source)

Building from source sidesteps the unsigned-binary trust question entirely — recommended if you're unsure about the pre-built download, or want the latest unreleased changes.

**Prerequisites**

| Tool | Version |
|---|---|
| CMake | 3.20+ |
| MinGW-w64 (Windows) or GCC/Clang (Linux/macOS) | C++23-capable |
| Git | any |

<details>
<summary><strong>Windows (MinGW)</strong></summary>

```bash
git clone https://github.com/PRITISH-TOMAR/spin_shell.git
cd spin_shell

cmake -G "MinGW Makefiles" -B build -S .
cmake --build build

./build/spin_shell
```

Or use the convenience script (cleans `build/` each time):

```bash
bash run.sh
```

</details>

<details>
<summary><strong>Linux / macOS (GCC/Clang)</strong></summary>

```bash
git clone https://github.com/PRITISH-TOMAR/spin_shell.git
cd spin_shell

cmake -B build -S .
cmake --build build

./build/spin_shell
```

> On macOS you may need `brew install cmake` and Xcode CLT (`xcode-select --install`).

</details>

**Optional: add to PATH**

```bash
# Linux / macOS
sudo cp build/spin_shell /usr/local/bin/spin_shell

# Windows — add build/ to your system PATH, or copy the binary:
copy build\spin_shell.exe C:\Windows\System32\spin_shell.exe
```

---

## Development

```bash
bash run.sh
```

---

## Roadmap — Toward a Full POSIX-Compliant Shell

spin_shell is being built out into a complete, scriptable command-line environment — covering control flow, expansion, interactive editing, job control, and scripting semantics found in mature shells like bash and zsh.

### Phase 2 — Core Shell Language

The foundational language layer: how commands chain together, how strings and variables are interpreted, and how the environment is managed. Every later feature (scripting, job control, advanced redirection) builds on this.

**Control Flow & Execution Chaining**
- [ ] `;` — sequential command execution
- [ ] `&&` — conditional execution on success
- [ ] `||` — conditional execution on failure
- [ ] `&` — asynchronous background execution

**Quoting, Escaping & Literal Handling**
- [ ] Single-quote literal strings (no expansion)
- [ ] Double-quote strings with embedded `$VAR` expansion
- [ ] Backslash escaping for special characters

**Expansion Engine**
- [ ] `${VAR}` — braced variable expansion
- [ ] `${VAR:-default}` — parameter expansion with defaults
- [ ] `$(cmd)` — command substitution
- [ ] `$((expr))` — arithmetic expansion

**Environment Management & Aliasing**
- [ ] `unset` — variable removal
- [ ] `env` — environment inspection
- [ ] `alias` / `unalias` — command aliasing system

### Phase 3 — Interactive & Scripting Capabilities

Builds on the Phase 2 language layer to deliver a real interactive experience and a full scripting runtime — the point where spin_shell stops being a command interpreter and becomes a shell people script and live in.

**Interactive Shell Experience (Readline-class UX)**
- [ ] Full history navigation via arrow keys
- [ ] Tab completion for commands and filesystem paths
- [ ] `Ctrl+R` incremental reverse history search
- [ ] Advanced line editing — home/end, word-jump navigation

**Scripting Language Support**
- [ ] `if / elif / else / fi` conditionals
- [ ] `for / while / until` loop constructs
- [ ] `case` pattern-matching statements
- [ ] User-defined `function`s
- [ ] `source` / `.` — script execution in current shell context
- [ ] `return` with exit-code propagation

**Process & Job Control**
- [ ] `jobs` — background job listing
- [ ] `fg` / `bg` — job state management
- [ ] Signal handling for `SIGINT` / `SIGTSTP` (`Ctrl+C` / `Ctrl+Z`)

**Advanced Shell Semantics**
- [ ] Here-documents (`<<EOF ... EOF`)
- [ ] Here-strings (`<<< "text"`)
- [ ] Process substitution (`<(cmd)`, `>(cmd)`)
- [ ] Subshells with isolated execution environments
- [ ] `exec` — process image replacement
- [ ] `trap` — signal handling in scripts

Each phase builds on the last, moving spin_shell from a functional command interpreter toward a shell capable of running real-world scripts, interactive workflows, and job-managed sessions end to end.

---

## Contributing

Issues and pull requests are welcome. If you're picking up an item from the roadmap, opening an issue first to flag what you're working on helps avoid duplicate effort.