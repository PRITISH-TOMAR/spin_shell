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
static string runSegment(const string &rawSegment, string &inputBuf, bool isLast, ShellState &state);

#ifdef _WIN32
// Windows: Spawn external process, wiring stdin/out via anomnymous HANDLEs
static string runExternalWindows(const string &path, vector<string> &args, string &inputBuf, bool isLast);

#else
// Linux/Mac: fork + dup2 + execv with anonymous pipe fds
static string runExternalUnix(const string &path, vector<string> &args, string &inputBuf, bool isLast);

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

        if(start == string::npos)continue; // skip empty segments ( may be double pipe)

        segment= segment.substr(start, end - start + 1);

        bool isLast = (i ==  segments.size()-1);

        // run segment :: inputBuf is replace by its stdOut or "" if last
        inputBuf = runSegment(segment, inputBuf, isLast, state);
    }
}
