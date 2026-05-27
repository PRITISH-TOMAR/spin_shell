# Command Implementation Roadmap

## Pending

### pipe `|` ⬅ PRIORITY 1

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
- [ ] Handle all edge cases above


### ls
- [ ] Default to `ShellState.currentDirectory` when no path arg given
- [ ] Support `-a` flag (show hidden files)
- [ ] Support `-l` flag (long listing format)
- [ ] Support `-la` / `-al` combined flags
- [ ] Colorize output: directories, executables, regular files

### mkdir
- [ ] Support single dir creation
- [ ] Support `-p` flag (create parent dirs recursively)
- [ ] Error on existing dir unless `-p` is passed

### touch
- [ ] Create file if it does not exist
- [ ] Update access and modification timestamps if file exists
- [ ] Support multiple file arguments

### rm
- [ ] Remove one or more files
- [ ] Support `-r` / `-rf` flag for recursive directory removal
- [ ] Support `-f` flag to suppress errors on missing files

### rmdir
- [ ] Remove empty directories
- [ ] Error if directory is not empty (suggest `rm -r` instead)
- [ ] Support multiple directory arguments

### history
- [ ] Store every executed command in `ShellState.commandHistory`
- [ ] `history` prints all entries with line numbers
- [ ] `history N` prints last N entries
- [ ] Hook into `prepareInputForDispatch` to record each command before dispatch

### exit
- [ ] `exit` with no args exits with code 0
- [ ] `exit N` exits with code N
- [ ] Set `ShellState.lastExitCode` before returning

### help
- [ ] List all known built-in commands
- [ ] Brief description per command

### clear
- [ ] Output ANSI escape code `\033[2J\033[H` to clear terminal
- [ ] Works cross-platform (ANSI supported on Windows 10+)

### cp
- [ ] Copy a file to a destination file or directory
- [ ] Support `-r` flag for recursive directory copy
- [ ] Preserve source filename when destination is a directory

### mv
- [ ] Move/rename a file or directory
- [ ] Error if source does not exist
- [ ] Overwrite destination by default; support `-n` to skip if exists

### echo
- [ ] Print arguments separated by spaces followed by newline
- [ ] Support `-n` flag to suppress trailing newline
- [ ] Expand shell variables (`$VAR`, `$?`) already handled upstream by `ShellState`

### grep
- [ ] Search for a pattern in one or more files
- [ ] Support `-i` flag (case-insensitive)
- [ ] Support `-n` flag (print line numbers)
- [ ] Support `-r` flag (recursive directory search)
- [ ] Read from stdin when no file argument is given

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
- [ ] `*`, `?`, `[...]` — glob/wildcard expansion in args

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
- [ ] `export VAR=val` — promote shell var to environment variable
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