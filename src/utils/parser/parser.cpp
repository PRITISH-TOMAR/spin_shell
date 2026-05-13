#include "parser.hpp"
#include <sstream>

ParsedInput parseInput(const string& input) {
    ParsedInput result;

    // 1. Tokenize the input by spaces
    istringstream stream(input);
    vector<string> tokens;
    string token;

    while (stream >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) return result;

    // 2. First token is always the command
    result.command = tokens[0];

    // 3. Rest are either flags or args
    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i][0] == '-') {
            result.flags.push_back(tokens[i]);  // starts with - flag
        } else {
            result.args.push_back(tokens[i]);   // otherwise  arg
        }
    }

    return result;
}