# System Calls 

#### 1. Fork <unistd.h>

- System call fork() is used to create processes. It takes no arguments and returns a process ID. 
- The purpose of fork() is to create a new process, which becomes the child process of the caller. 
- After a new child process is created, both processes will execute the next instruction following the fork() system call.

- If fork() returns a negative value, the creation of a child process was unsuccessful.
- fork() returns a zero to the newly created child process.
- fork() returns a positive value, the process ID of the child process, to the parent. 

- The returned process ID is of type pid_t defined in **sys/types.h**. Normally, the process ID is an integer. Moreover, a process can use function getpid() to retrieve the process ID assigned to this process.

#### 2. 