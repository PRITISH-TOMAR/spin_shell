# Command Implementation Roadmap

## Pending


### ls
- [ ] Default to `ShellState.currentDirectory` when no path arg given
- [ ] Support `-a` flag (show hidden files)
- [ ] Support `-l` flag (long listing format)
- [ ] Support `-la` / `-al` combined flags
- [ ] Colorize output: directories, executables, regular files

### history
- [ ] Store every executed command in `ShellState.commandHistory`
- [ ] `history` prints all entries with line numbers
- [ ] `history N` prints last N entries
- [ ] Hook into `prepareInputForDispatch` to record each command before dispatch

### clear
- [ ] Output ANSI escape code `\033[2J\033[H` to clear terminal
- [ ] Works cross-platform (ANSI supported on Windows 10+)

### mkdir
- [ ] Support single dir creation
- [ ] Support `-p` flag (create parent dirs recursively)
- [ ] Error on existing dir unless `-p` is passed

