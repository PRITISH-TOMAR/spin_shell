#include "pipe_utils.hpp"


vector<string> splitOnPipe(const string &input)
{

    vector<string> segments;
    string current;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;

    for (size_t i = 0; i < input.size(); i++)
    {
        char c = input[i];

        if (c == '\'' && !inDoubleQuote)
            inSingleQuote = !inSingleQuote;
        else if (c == '\"' && !inSingleQuote)
            inDoubleQuote = !inDoubleQuote;
        else if (c == '|' && !inSingleQuote && !inDoubleQuote)
        {
            segments.push_back(current);
            current.clear();
            continue;
        }
        current += c;
    }
    segments.push_back(current);
    return segments;
}

bool containsPipe(const string &input)
{

    bool inSingle = false, inDouble = false;
    for (char c : input)
    {
        if (c == '\'' && !inDouble)
            inSingle = !inSingle;
        else if (c == '"' && !inSingle)
            inDouble = !inDouble;
        else if (c == '|' && !inSingle && !inDouble)
            return true;
    }
    return false;
}