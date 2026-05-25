#include "pipe_runner.hpp"
#include "pipe_utils.hpp"

#include "src/commands/commands.hpp"
#include "src/utils/handlers/input_handler.hpp"
#include "src/utils/findInPath.hpp"
#include "src/commands/dispatch.hpp"

#include <sstream>
#include <iostream>
// Buitin/toool handler wired in main.cpp
// ALWAYS in Sync with main.cpp SWITCH BLOCK
using namespace std;

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

// Run 1 segment
static string runSegment(const string &rawSegment, const string &inputBuf, bool isLast, ShellState &state);

#ifdef _WIN32
// Windows: Spawn external process, wiring stdin/out via anonymous HANDLEs
static string runExternalWindows(const string &path, const vector<string> &args, const string &inputBuf, bool isLast);

#else
// Linux/Mac: fork + dup2 + execv with anonymous pipe fds
static string runExternalUnix(const string &path, const vector<string> &args, const string &inputBuf, bool isLast);

#endif

// Enry point to be called from main.cpp -> run pipeline
void runPipeline(const vector<string> &segments, ShellState &state)
{

    // inputBuf threads through the loop.

    string inputBuf;

    for (size_t i = 0; i < segments.size(); i++)
    {

        string segment = segments[i];
        size_t start = segment.find_first_not_of(" \t");
        size_t end = segment.find_last_not_of(" \t");

        if (start == string::npos)
            continue; // skip empty segments ( may be double pipe)

        segment = segment.substr(start, end - start + 1);

        bool isLast = (i == segments.size() - 1);

        // run segment :: inputBuf is replace by its stdOut or "" if last
        inputBuf = runSegment(segment, inputBuf, isLast, state);
    }
}

static string runSegment(const string &rawSegment, const string &inputBuf, bool isLast, ShellState &state)
{
    ParsedInput parsed;
    string mutableSegment = rawSegment;

    if (prepareInputForDispatch(mutableSegment, state, parsed))
    {
        // pure assignment ("x=value")
        return inputBuf;
    }

    Command cmd = Command::UNKNOWN;

    auto existCommand = commandMap.find(parsed.command);
    if (existCommand != commandMap.end())
    {
        cmd = existCommand->second;
    }

    if (cmd == Command::EXIT)
    {
        return inputBuf;
    }

    // Swap the underlying rdBuf pointers so it transparently redas from
    // inputBuf and write to ostringStream we can capture.
    if (cmd != Command::UNKNOWN)
    {

        // feed inputBuf (input) into the handler as if it is a normal stdin
        istringstream inStream(inputBuf);

        streambuf *savedCin = cin.rdbuf(inStream.rdbuf());

        ostringstream outStream;

        streambuf *savedCout = nullptr;

        if (!isLast)
            savedCout = cout.rdbuf(outStream.rdbuf());

        dispatchCommand(cmd, parsed, state);

        // Always restore cin
        cin.rdbuf(savedCin);

        if (!isLast)
        {
            cout.rdbuf(savedCout);  // restore real cout
            return outStream.str(); // output before next input
        }

        return ""; // last Segment direct write to terminal
    }

    // External Commands path ------------------------------------------
    string path = findInPath(parsed.command);

    if (path.empty())
    {
        cerr << parsed.command << ": command not found\n";
        return "";
    }

#ifdef _WIN32
    return runExternalWindows(path, parsed.rawArgs, inputBuf, isLast);
#else
    return runExternalUnix(path, parsed.rawArgs, inputBuf, isLast);
#endif
}

static string runExternalWindows(const string &path, const vector<string> &args, const string &inputBuf, bool isLast)
{
    HANDLE hStdinRead = nullptr, hStdinWrite = nullptr;
    HANDLE hStdoutRead = nullptr, hStdoutWrite = nullptr;

    // Pipe Handle must be inheritable for child process to use them
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    // create stdin pipe; child reads from hStdinRead
    if (!CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0))
        return "";

    // The write end is for parent only -- do NOT inherit it, or the child will block waiting for more input even after the parent finishes writing
    SetHandleInformation(hStdinWrite, HANDLE_FLAG_INHERIT, 0);

    // create stdout pipe only when we need to capture (non-last segment)
    if (!isLast)
    {
        if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
        {
            CloseHandle(hStdinRead);
            CloseHandle(hStdinWrite);
            return "";
        }

        // Read end is for parent only -- do NOT inherit it
        SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);
    }

    // Build the command-line string expected by CreateProcess: "path" "arg1" "arg2"
    // Fix: each arg needs a leading space, otherwise they concatenate: "path""arg1"
    string cmdLineStr = "\"" + path + "\"";
    for (const auto &arg : args)
        cmdLineStr += " \"" + arg + "\"";

    // CreateProcessA may modify the buffer, so copy into a writable vector<char>
    // instead of passing c_str() (read-only) via const_cast — that would be UB
    vector<char> cmdLine(cmdLineStr.begin(), cmdLineStr.end());
    cmdLine.push_back('\0');

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = hStdinRead;
    si.hStdOutput = isLast ? GetStdHandle(STD_OUTPUT_HANDLE) // real console
                           : hStdoutWrite;                   // capture pipe
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);          // always real stderr

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(
        nullptr,
        cmdLine.data(), // writable buffer
        nullptr, nullptr,
        TRUE,           // bInheritHandles: child gets copies of inheritable handles
        0, nullptr, nullptr,
        &si, &pi
    );

    // Close parent's copies of the child-side pipe ends immediately after spawn.
    // Leaving them open prevents EOF: child's ReadFile/WriteFile never unblocks.
    CloseHandle(hStdinRead);
    if (!isLast) CloseHandle(hStdoutWrite);

    if (!ok)
    {
        if (!isLast) CloseHandle(hStdoutRead);
        CloseHandle(hStdinWrite);
        cerr << "CreateProcess failed (error " << GetLastError() << ")\n";
        return "";
    }

    // Write inputBuf to child's stdin, then close the write end to signal EOF.
    // Without closing, the child blocks forever waiting for more input.
    if (!inputBuf.empty())
    {
        DWORD written = 0;
        WriteFile(hStdinWrite, inputBuf.data(),
                  static_cast<DWORD>(inputBuf.size()), &written, nullptr);
    }
    CloseHandle(hStdinWrite);

    // Read child's stdout into outputBuf (only for non-last segments).
    // ReadFile blocks until data arrives or the write end closes (child exits).
    string outputBuf;
    if (!isLast)
    {
        char buf[4096];
        DWORD bytesRead = 0;
        while (ReadFile(hStdoutRead, buf, sizeof(buf), &bytesRead, nullptr) && bytesRead > 0)
            outputBuf.append(buf, bytesRead);
        CloseHandle(hStdoutRead);
    }

    // Wait for child to fully exit before returning so the next segment
    // does not start reading outputBuf before the child has finished writing it.
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return outputBuf; // "" when isLast — child wrote directly to the console
}