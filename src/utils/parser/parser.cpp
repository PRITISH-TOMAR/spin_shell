#include "parser.hpp"

ParsedInput parseInput(const string &input)
{
    ParsedInput parsed;
    size_t i = 0, n = input.size();
    bool first = true;

    while (i < n) {
        while (i < n && isspace((unsigned char)input[i])) ++i;
        if (i >= n) break;

        string token;
        while (i < n && !isspace((unsigned char)input[i])) {
            char c = input[i];
            if (c == '\'' || c == '"') {
                ++i;
                while (i < n && input[i] != c) token += input[i++];
                if (i < n) ++i;
            } else {
                token += c; ++i;
            }
        }

        if (first) { parsed.command = token; first = false; continue; }

        parsed.rawArgs.push_back(token);

        if (token.size() > 1 && token[0] == '-' && token[1] != '-') {
            for (size_t j = 1; j < token.size(); j++)
                parsed.shortFlags[token[j]] = true;
        } else if (token.size() > 2 && token.substr(0, 2) == "--") {
            parsed.longFlags[token.substr(2)] = true;
        } else {
            parsed.files.push_back(token);
        }
    }
    return parsed;
}

// input: "grep 'hello world' -i --count file.txt"
//
// tokenize() → ["grep", "hello world", "-i", "--count", "file.txt"]
//                              ↑ quotes stripped, space preserved
//
// Final ParsedInput:
// {
//     command    = "grep"
//     rawArgs    = ["hello world", "-i", "--count", "file.txt"]
//     shortFlags = { 'i':true }
//     longFlags  = { "count":true }
//     files      = ["hello world", "file.txt"]
// }



// determine if a flag is present in the parsed input
bool hasFlag(const ParsedInput& parsed, char s, const string& l) {
    return parsed.shortFlags.count(s) || parsed.longFlags.count(l);
}
