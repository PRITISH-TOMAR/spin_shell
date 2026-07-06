# Command Implementation Roadmap

## Next Steps

### Phase 1 — CI / GitHub Actions
- [x] `.github/workflows/ci.yml` — Build + smoke tests on Windows/Linux/macOS (push & PR to main) ✓
- [ ] `.github/workflows/release.yml` — Tag-triggered release: builds binaries, checksums, GitHub Release
- [ ] `tests/smoke.sh` — Smoke test script piped into the binary (echo, pwd, exit codes)
- [ ] `.github/dependabot.yml` — Weekly auto-update PRs for GitHub Actions versions

### Phase 2 — GitHub Releases & Binaries
- [ ] Add a `release` workflow that triggers on `git tag v*`
- [ ] Build binaries for all targets in CI:
  - Windows x64 (MinGW)
  - Linux x86_64 (GCC)
  - macOS arm64 / x86_64 (Clang)
- [ ] Upload built binaries as GitHub Release artifacts automatically
- [ ] Include checksums (`sha256sum`) for each binary in the release notes

### Phase 3 — Install Script
- [ ] Write `install.sh` (Linux/macOS):
  - Detects OS and architecture
  - Downloads the correct binary from latest GitHub Release
  - Places it in `/usr/local/bin/myshell`
  - Appends to `/etc/shells` so it can be set as login shell
- [ ] Write `install.ps1` (Windows):
  - Downloads `.exe` from latest GitHub Release
  - Places it in a sensible location (e.g. `C:\Program Files\myshell\`)
  - Adds that directory to the user's `PATH`
- [ ] Document one-liner install in README:
  - `curl -fsSL https://raw.githubusercontent.com/.../install.sh | sh`

### Phase 4 — Package Managers
- [ ] Submit `winget` manifest (Windows)
- [ ] Write `brew` formula and submit to Homebrew (macOS)
- [ ] Create AUR `PKGBUILD` (Arch Linux)
- [ ] Create `.deb` package (Debian/Ubuntu)
- [ ] Create `.rpm` spec (Fedora/RHEL)

### Phase 5 — Login Shell Registration
- [ ] Document how to set as default shell post-install:
  - Linux/macOS: `chsh -s /usr/local/bin/myshell`
  - Windows: set as default terminal profile in Windows Terminal

---

## Pending

### ls
- [ ] Colorize output: directories, executables, regular files

### glob expansion
- [ ] Support `[...]` character-class patterns

---

## Future Features

### Phase 3 — Control Operators
- [ ] `;` — sequential execution (`cmd1; cmd2`)
- [ ] `&&` — run next only if previous succeeded
- [ ] `||` — run next only if previous failed
- [ ] `&` — background execution

### Phase 4 — Quoting & Escaping
- [ ] Single quotes `'...'` — literal, no expansion
- [ ] Double quotes `"..."` — allow `$VAR` expansion inside
- [ ] Backslash `\` — escape next character

### Phase 5 — Variable & Command Expansion
- [ ] `${VAR}` — braced variable expansion
- [ ] `${VAR:-default}` — default value if unset
- [ ] `$(cmd)` — command substitution
- [ ] `$((expr))` — arithmetic expansion

### Phase 6 — Readline / Interactive UX
- [ ] Arrow key history navigation (up/down)
- [ ] Tab completion for commands and paths
- [ ] `Ctrl+R` reverse history search
- [ ] Line editing: home/end, word-jump (`Ctrl+Left/Right`)

### Phase 7 — Shell Scripting
- [ ] `if / elif / else / fi`
- [ ] `for / while / until` loops
- [ ] `case` statements
- [ ] `function` definitions
- [ ] `source` / `.` — execute script in current shell context
- [ ] `return` with exit code

### Phase 8 — Job Control
- [ ] `jobs` — list background jobs
- [ ] `fg` / `bg` — foreground/background a job
- [ ] `Ctrl+C` / `Ctrl+Z` signal handling (`SIGINT`, `SIGTSTP`)

### Phase 9 — Environment & Aliases
- [ ] `unset VAR` — remove a variable
- [ ] `env` — print all environment variables
- [ ] `alias` / `unalias` — define and remove command aliases

### Phase 10 — Advanced
- [ ] Here-documents (`<<EOF ... EOF`)
- [ ] Here-strings (`<<< "text"`)
- [ ] Process substitution (`<(cmd)`, `>(cmd)`)
- [ ] Subshell `(cmd1; cmd2)` — isolated environment
- [ ] `exec` — replace shell process with a command
- [ ] `trap` — handle signals in scripts

---
