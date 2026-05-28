# System Calls

#### 1. Fork <unistd.h>

- System call fork() is used to create processes. It takes no arguments and returns a process ID.
- The purpose of fork() is to create a new process, which becomes the child process of the caller.
- After a new child process is created, both processes will execute the next instruction following the fork() system call.

- If fork() returns a negative value, the creation of a child process was unsuccessful.
- fork() returns a zero to the newly created child process.
- fork() returns a positive value, the process ID of the child process, to the parent.

- The returned process ID is of type pid_t defined in **sys/types.h**. Normally, the process ID is an integer. Moreover, a process can use function getpid() to retrieve the process ID assigned to this process.

#### 2. Pipe <InterProcess_Communication>

A one way data channel inside the kernel with two ends:

```
WRITE end ─────────────► KERNEL BUFFER ─────────────► READ end
```

##### File Descriptor (fd)

- Just an integer that represents an open resource in your process.

```
Every process starts with 3 by default:
  fd 0 → stdin  (keyboard)
  fd 1 → stdout (terminal)
  fd 2 → stderr (terminal)
```

```
pipe(fd)
  fd[0] = read  end
  fd[1] = write end

fork()
  Parent: fd[0], fd[1]
  Child:  fd[0], fd[1]

── Child ──────────────────────
close(fd[0])        // child won't read
write(fd[1], &x)    // child writes x into pipe
close(fd[1])        // signals EOF to parent

── Parent ─────────────────────
close(fd[1])        // parent won't write
read(fd[0], &x)     // parent reads what child wrote ✓
close(fd[0])        // done reading
```

```
Parent forgets close(fd[1])
  → parent still holds write end open
  → read(fd[0]) blocks forever
  → never sees EOF
  → DEADLOCK
```

##### A Deadlock Behaviour ..

```
Parent                KERNEL BUFFER          Child
──────                ─────────────          ─────
                                          write(fd[1], "hello")
                         [hello]
                                          close(fd[1])
                      write end still
                      open (parent's
                      fd[1] not closed)
read(fd[0]) ◄── waiting ────────────── EOF never comes
     │
     blocks
     forever
     ↓
  DEADLOCK
```
