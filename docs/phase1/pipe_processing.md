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

shell as a parent command have to invoke a child command