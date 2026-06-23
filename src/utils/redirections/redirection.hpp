#pragma once
#include<string>
#include<vector>

using namespace std;

enum class RedirectionType{

    STDOUT_OVERWRITE, // >,
    STDOUT_APPEND, // >>,
    STDIN_FILE, // <,
    STDERR_FILE, // 2>,
    STDERR_TO_STDOUT, //2>&1,
    BOTH_FILE, // &>,
};

struct Redirection{
    RedirectionType type;
    string file; // empty for STDERR_TO_STDOUT
};

void parseRedirections(string &raw, vector<Redirection> & out);
