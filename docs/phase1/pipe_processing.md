# Pipe Processing

## Flow of piping throughout the module

```
pipe_utils.cpp
```

### containsPipe

Scans input char-by-char , respect quotes.
If pipe not in quotes, then returns true.

### splitOnPipe

Same scan as "containsPipe" but returns a vector<string> of segments(sub-commands).

---

```
pipe_runner.cpp
```

### runPipeLine

- iterate through all segments and pass output of previous command into input of current command.

---

### runSegment

- fetch the command type - External or Inbuilt

- If External, **runExternalWindows** or **runExternalUnix**

- If Inbuilt
  - StringStream creates a fake input stream st looks like it arrives from keyboard(standard cin).
  - savedCin forces the current command to take input from Stringstream stream.
  - savedCout creates a pointer out stream such that whatever appears in terminal must be stored in savedCout.
  - actual command execution occurs in dispatchCommand.
  - whatever command output is it is stored in savedCout.
  - Always, restore cin and cout as terminal.

---

### runExternalWindows

shell as a parent processes have to invoke a child processes otherwise it will exit without executing further commands.

```
OS Handle Table for PID <shell_pid>:
  Handle 0 → Keyboard (stdin)
  Handle 1 → Terminal (stdout)
  Handle 2 → Terminal (stderr)
```

- Any handle created with this security attribute can be copied into a child process when that child is spawned.

- Windows creates a pipe — a one-way data channel inside the kernel:

```
KERNEL           ┌──────────┐
hStdinWrite ────►│  Buffer  │────►  hStdinRead
(write end)      │          │      (read end)
                 └──────────┘
```

Updated OS handle table for PID<shell_pid>

```OS Handle Table for PID 1000:
  Handle 0   → Keyboard
  Handle 1   → Terminal
  Handle 2   → Terminal
  Handle 100 → Pipe1 Read  (hStdinRead)  [INHERITABLE]
  Handle 101 → Pipe1 Write (hStdinWrite) [INHERITABLE]
```

Remove inherit flag from hStdinWrite

```
Child would own:
  - hStdinRead  (its stdin)
  - hStdinWrite (a write end to its OWN stdin)

Result:
  Parent closes hStdinWrite after writing
  BUT child still has hStdinWrite open
  So pipe NEVER gets EOF
  Child blocks forever on ReadFile
  ← DEADLOCK
```

Create stdout Pipe (isLast = false)

```
KERNEL
                 ┌──────────┐
hStdoutWrite ───►│  Buffer  │────► hStdoutRead
(child writes)   │          │      (parent reads)
                 └──────────┘
```

HANDLE TABLE now

```
OS Handle Table for PID 1000:
  Handle 0   → Keyboard
  Handle 1   → Terminal
  Handle 2   → Terminal
  Handle 100 → Pipe1 Read  (hStdinRead)  [INHERITABLE ✓]
  Handle 101 → Pipe1 Write (hStdinWrite) [INHERITABLE ✗]
  Handle 102 → Pipe2 Read  (hStdoutRead) [INHERITABLE ✗]
  Handle 103 → Pipe2 Write (hStdoutWrite)[INHERITABLE ✓]
```

Only hStdinRead and hStdoutWrite are inheritable — these are the two ends the child needs.

NEED FOR 2 PIPEs

```
Shell needs to SEND data  →  to child's stdin
Shell needs to RECEIVE data  ←  from child's stdout
```

---

BUILD command line

```
cmdLine = [ '"', 'C', ':', '/', 'b', 'i', 'n', '/', 'l', 's', '.', 'e', 'x', 'e', '"', '\0' ]
```

**STARTUPINFOA**
Child will be born with:
its stdin = Pipe1 Read ← parent can feed data in
its stdout = Pipe2 Write ← parent can capture output
its stderr = Terminal ← errors go straight to screen

**CreateProcessA**: Actual child process get alive

```
PID 1000 (Shell)              PID 2000 (ls.exe)
────────────────              ─────────────────
Handle 100 → Pipe1 Read       Handle 100 → Pipe1 Read  ← same pipe
Handle 101 → Pipe1 Write      Handle 103 → Pipe2 Write ← same pipe
Handle 102 → Pipe2 Read
Handle 103 → Pipe2 Write

Both running simultaneously now!
```

Close child-side handles in PARENT

- shell's Handle 103 is STILL OPEN

  So when parent does:
  ReadFile(hStdoutRead, ...) ← waiting for EOF

  Windows says: "Pipe2 write end still open (shell has it)"
  "Not EOF yet, keep waiting..."

  Parent hangs FOREVER ← DEADLOCK

ACTUAL CHILD PROCESS EXECUTION BEGINS..

- Write inputBuf to child & Close WriteHandle
- Parent reads output from child and Close Readhandle

Wait and Cleanup, parent blocked untill child executes

Return the output buffer successfully which becomes input for next command

---

---

### runExternalUnix -- trace command : ls | grep .txt

```
Shell Process (PID 1000)
─────────────────────────────
Memory:
  inputBuf = ""
  path     = "/bin/ls"
  isLast   = false

File Descriptor Table for PID 1000:
  fd 0 → Keyboard (stdin)
  fd 1 → Terminal (stdout)
  fd 2 → Terminal (stderr)
```

- Unix uses file descriptors (fd) — just integers. Windows used named HANDLE objects.

- pipe() creates two file descriptors:

```
KERNEL
                 ┌──────────┐
stdinPipe[1] ───►│  Buffer  │────► stdinPipe[0]
  fd 4           │          │        fd 3
  (parent        └──────────┘      (child reads
   writes)                          from here)
```

File Descriptor now:

```
FD Table PID 1000:
  fd 0 → Keyboard
  fd 1 → Terminal
  fd 2 → Terminal
  fd 3 → stdinPipe  read end
  fd 4 → stdinPipe  write end
```

- create stdoutPipe => isLast = false

```
KERNEL
                 ┌──────────┐
stdoutPipe[1] ──►│  Buffer  │────► stdoutPipe[0]
  fd 6           │          │        fd 5
  (child         └──────────┘      (parent reads
   writes)                          from here)
```

- File Descriptor now

```
FD Table PID 1000:
  fd 0 → Keyboard
  fd 1 → Terminal
  fd 2 → Terminal
  fd 3 → stdinPipe  read  end
  fd 4 → stdinPipe  write end
  fd 5 → stdoutPipe read  end
  fd 6 → stdoutPipe write end
```

** Build argv[]** == argv = [ "/bin/ls", nullptr ]

#### Fork dupicates the entire process

```
BEFORE fork():
  PID 1000 (shell)
  fd 0,1,2,3,4,5,6
  memory: path, args, inputBuf...

AFTER fork():
  PID 1000 (shell/parent)     PID 2000 (exact copy/child)
  fd 0,1,2,3,4,5,6            fd 0,1,2,3,4,5,6  ← same pipes!
  memory: same                memory: same copy
  pid = 2000                  pid = 0
```

```
BEFORE dup2:
fd 0 → Keyboard
fd 3 → stdinPipe read end

AFTER dup2(3, 0):
fd 0 → stdinPipe read end  ← now stdin reads from pipe!
fd 3 → stdinPipe read end  ← duplicate, no longer needed
```

```
close(stdinPipe[0]);  // fd 3 — original, no longer needed
close(stdinPipe[1]);  // fd 4 — child never writes to its own stdin
```

- dup2(oldfd, newfd) — makes newfd point to the same thing as oldfd:

```
BEFORE dup2:
  fd 1 → Terminal
  fd 6 → stdoutPipe write end

AFTER dup2(6, 1):
  fd 1 → stdoutPipe write end  ← stdout now writes to pipe!
  fd 6 → stdoutPipe write end  ← duplicate, no longer needed
```

```
ls runs:
reads from fd 0 (stdin)   → stdinPipe  (empty, ls ignores it)
writes to fd 1 (stdout)   → stdoutPipe (captured by parent!)
writes to fd 2 (stderr)   → Terminal
```

- Parent gets alive here

- close(stdinPipe[0]); // fd 3 — parent never reads from stdin pipe

```
PARENT"S FD TABLE NOW AFTER CLOSE

FD Table PID 1000:
  fd 0 → Keyboard
  fd 1 → Terminal
  fd 2 → Terminal
  fd 4 → stdinPipe  write end  ← to feed input to child
  fd 5 → stdoutPipe read end   ← to read output from child
  fd 6 → stdoutPipe write end  ← will close soon
```

Write inputBuf to child

** WHAT HAPPENED SIMULTANEOUSLY**

```
PID 1000 (Shell)                  PID 2000 (ls)
────────────────                  ─────────────
read(stdoutPipe[0])               lists directory
  blocks...                       write(fd1): "main.cpp\n"
  ← gets "main.cpp\n"             write(fd1): "fin.txt\n"
  ← gets "fin.txt\n"              write(fd1): "notes.txt\n"
  ← gets "notes.txt\n"            write(fd1): "build.sh\n"
  ← gets "build.sh\n"            exits → fd1 closes → EOF
  ← read() returns 0
  loop ends
```

```
return outputBuf;
// "main.cpp\nfin.txt\nnotes.txt\nbuild.sh\n"
```


### UNIX VS WINDOWS

| Step | Windows | Unix |
|------|---------|------|
| Create pipe | `CreatePipe()` | `pipe()` |
| Spawn process | `CreateProcessA()` | `fork()` + `execv()` |
| Redirect stdin | `STARTUPINFO.hStdInput` | `dup2(stdinPipe[0], 0)` |
| Redirect stdout | `STARTUPINFO.hStdOutput` | `dup2(stdoutPipe[1], 1)` |
| Write to child | `WriteFile()` | `write()` |
| Read from child | `ReadFile()` | `read()` |
| Wait for child | `WaitForSingleObject()` | `waitpid()` |
| Prevent zombie | not needed | `waitpid()` |
| Handle type | `HANDLE` (object) | `int` (fd number) |
