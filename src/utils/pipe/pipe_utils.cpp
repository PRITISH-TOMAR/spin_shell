#include "pipe_utils.hpp"
#include "src/utils/quote_utils.hpp"

vector<string> splitOnPipe(const string &input)
{
    vector<string> segments;
    string current;
    QuoteTracker qt;

    for (char c : input)
    {
        qt.update(c);
        if (c == '|' && !qt.inQuotes())
        {
            segments.push_back(current);
            current.clear();
        }
        else
        {
            current += c;
        }
    }
    segments.push_back(current);
    return segments;
}

bool containsPipe(const string &input)
{
    QuoteTracker qt;
    for (char c : input)
    {
        qt.update(c);
        if (c == '|' && !qt.inQuotes())
            return true;
    }
    return false;
}