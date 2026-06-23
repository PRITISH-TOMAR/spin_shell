# Command Implementation Roadmap

## Pending

### pipe `|`

#### Concept
A pipe connects the **stdout of one command to the stdin of the next**.
The shell splits the raw input into segments on `|`, runs each segment in order,
and feeds the output of each as the input of the next.

```
echo "hello world" | grep "hello" | cat
      ↓ stdout           ↓ stdout
                ↓ stdin            ↓ stdin
```

#### Flow (per pipeline execution)

```
raw input: "cmd1 | cmd2 | cmd3"
              │
              ▼
        containsPipe()          ← detect unquoted '|'
              │
              ▼
        splitOnPipe()           ← ["cmd1", "cmd2", "cmd3"]
              │
              ▼
        runPipeline(segments)
              │
    ┌─────────┴──────────────────────────┐
    │  for each segment i:               │
    │                                    │
    │   prepareInputForDispatch()        │ ← trim, expand vars, parse
    │         │                          │
    │   is builtin?                      │
    │    YES → redirect cin/cout         │ ← rdbuf swap (no handler change needed)
    │           dispatchCommand()        │
    │           restore cin/cout         │
    │    NO  → external process          │
    │           Windows: CreateProcess   │ ← stdin/stdout wired via HANDLE
    │           Linux:   fork+dup2+exec  │ ← stdin/stdout wired via fd
    │                                    │
    │   outputBuf ──────────────────────►│ becomes inputBuf for segment i+1
    └────────────────────────────────────┘
              │
    last segment → output goes to real stdout
```

#### Two-layer strategy
| Command type | stdin source | stdout destination | Method |
|---|---|---|---|
| Builtin (not last) | `istringstream(inputBuf)` via `cin.rdbuf()` | `ostringstream` via `cout.rdbuf()` | C++ stream swap |
| Builtin (last) | `istringstream(inputBuf)` via `cin.rdbuf()` | real `cout` | C++ stream swap |
| External (not last) | anonymous pipe write → child stdin | anonymous pipe read → `outputBuf` | OS pipe + process |
| External (last) | anonymous pipe write → child stdin | real console handle | OS pipe + process |

#### Windows: CreateProcess pipe wiring
```
Parent writes inputBuf → hStdinWrite → [pipe] → hStdinRead → child stdin
Child stdout → hStdoutWrite → [pipe] → hStdoutRead → Parent reads → outputBuf
```

#### Linux: fork + dup2 pipe wiring (follow-up)
```
pipe(fds)                        // fds[0]=read end, fds[1]=write end
fork()
  child:  dup2(fds[1], STDOUT_FILENO)   // child stdout → pipe write end
          dup2(prev_read, STDIN_FILENO)  // child stdin  ← previous pipe read end
          execv(...)
  parent: close(fds[1])                 // close write end in parent
          read from fds[0] into buf     // capture output
```

#### Edge cases to handle
- Leading/trailing `|` → produces empty segment → skip silently
- Double `||` → empty middle segment → skip silently
- `|` inside single/double quotes → NOT a pipe separator
- `EXIT` inside a pipeline → no-op (only exits at top-level REPL)
- Command not found in a segment → print error, pass empty string forward

#### Checklist
- [x] `splitOnPipe()` — split on unquoted `|`
- [x] `containsPipe()` — fast pre-check before splitting
- [x] `pipe_utils.hpp / pipe_utils.cpp` — created
- [x] `pipe_runner.hpp` — created
- [x] `pipe_runner.cpp` — implement `runSegment` + `runPipeline`
- [x] Wire `containsPipe` + `runPipeline` into `main.cpp` REPL loop
- [x] Windows: `CreateProcess`-based external segment runner
- [x] Linux: `fork()`+`dup2()`+`pipe()` external segment runner
- [x] Handle all edge cases above


### ls
- [x] Default to `ShellState.currentDirectory` when no path arg given
- [x] Support `-a` flag (show hidden files)
- [x] Support `-l` flag (long listing format)
- [x] Support `-la` / `-al` combined flags
- [ ] Colorize output: directories, executables, regular files

### mkdir
- [x] Support single dir creation
- [x] Support `-p` flag (create parent dirs recursively)
- [x] Error on existing dir unless `-p` is passed

### touch
- [x] Create file if it does not exist
- [x] Update access and modification timestamps if file exists
- [x] Support multiple file arguments

### rm
- [x] Remove one or more files
- [x] Support `-r` / `-rf` flag for recursive directory removal
- [x] Support `-f` flag to suppress errors on missing files

### rmdir
- [x] Remove empty directories
- [x] Error if directory is not empty (suggest `rm -r` instead)
- [x] Support multiple directory arguments

### history
- [x] Store every executed command in `ShellState.commandHistory`
- [x] `history` prints all entries with line numbers
- [x] `history N` prints last N entries
- [x] Hook into REPL loop to record each command before dispatch

### exit
- [x] `exit` with no args exits with code 0
- [x] `exit N` exits with code N
- [x] Set `ShellState.lastExitCode` before returning

### help
- [x] List all known built-in commands
- [x] Brief description per command

### clear
- [x] Output ANSI escape code `\033[2J\033[H` to clear terminal
- [x] Works cross-platform (ANSI supported on Windows 10+)

### cp
- [x] Copy a file to a destination file or directory
- [x] Support `-r` flag for recursive directory copy
- [x] Preserve source filename when destination is a directory

### mv
- [x] Move/rename a file or directory
- [x] Error if source does not exist
- [x] Overwrite destination by default; support `-n` to skip if exists

### echo
- [x] Print arguments separated by spaces followed by newline
- [x] Support `-n` flag to suppress trailing newline
- [x] Expand shell variables (`$VAR`, `$?`) already handled upstream by `ShellState`

### grep
- [x] Search for a pattern in one or more files
- [x] Support `-i` flag (case-insensitive)
- [x] Support `-n` flag (print line numbers)
- [x] Support `-r` flag (recursive directory search)
- [x] Read from stdin when no file argument is given

### type
- [x] Identify shell builtins vs external executables
- [x] Print `name is a shell builtin` for known commands
- [x] Print `name is /path/to/exe` for commands found in `$PATH`
- [x] Print error and exit 1 for unknown commands
- [x] Support multiple arguments

### export
- [x] `export VAR=val` — set shell variable and promote to environment
- [x] `export VAR` — promote an already-defined shell variable to environment
- [x] `export` with no args — list all shell variables (`declare -x` format)

### glob expansion
- [x] Expand unquoted `*` and `?` in arguments against the current directory
- [x] Sort matches alphabetically
- [x] Keep literal token when no matches found (nullglob-off behaviour)
- [x] Skip hidden files (names starting with `.`) in expansion
- [ ] Support `[...]` character-class patterns

---

## Future Features

### Phase 2 — I/O Redirection
- [ ] `>` — redirect stdout to file (overwrite)
- [ ] `>>` — redirect stdout to file (append)
- [ ] `<` — redirect stdin from file
- [ ] `2>` — redirect stderr to file
- [ ] `2>&1` — merge stderr into stdout
- [ ] `&>` — redirect both stdout and stderr

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
- [x] `*`, `?` — glob/wildcard expansion in unquoted args
- [ ] `[...]` — character-class glob patterns

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
- [x] `export VAR=val` — promote shell var to environment variable
- [x] `export VAR` — promote already-defined shell var to environment
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
