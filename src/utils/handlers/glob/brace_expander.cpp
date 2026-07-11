#include "brace_expander.hpp"
#include <cctype>
#include <algorithm>
using std::string;
using std::vector;
using std::stoi;
using std::to_string;


// Top-level operator detection: Pattern OP Pattern

// Describes the first top-level operator found in a string, if any.
struct OperatorSplit
{
    bool   found;
    size_t pos;   // index where the operator starts
    size_t len;   // 1 for ',', 2 for ".."
    char   type;  // ',' or '.'
};


// Finds the closing '}' matching the '{' at openPos, tracking nesting depth.
static size_t findMatchingBrace(const string &s, size_t openPos)
{
    if (openPos >= s.size() || s[openPos] != '{')
        return string::npos;

    int depth = 0;
    for (size_t i = openPos; i < s.size(); ++i)
    {
        if (s[i] == '{') ++depth;
        if (s[i] == '}') --depth;
        if (depth == 0) return i;
    }
    return string::npos;
}


// Scans `s` left-to-right at depth 0 (ignoring nested { }) for the first
// ',' or ".." — whichever occurs first. This is what drives the grammar:
// Example: "{a,b},c"  → skips comma inside { } (depth>0) → finds ',' at pos 5
static OperatorSplit findTopLevelOperator(const string &s)
{
    int depth = 0;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '{') { ++depth; continue; }
        if (s[i] == '}') { --depth; continue; }
        if (depth != 0) continue; // only look at top-level chars

        if (s[i] == ',')
            return { true, i, 1, ',' };

        if (s[i] == '.' && i + 1 < s.size() && s[i + 1] == '.')
            return { true, i, 2, '.' };
    }
    return { false, 0, 0, 0 }; // no operator found → atom
}

// Sequence (".." operator) value generation

static bool looksNumeric(const string &from, const string &to)
{
    return !from.empty() && !to.empty() &&
           (std::isdigit((unsigned char)from[0]) || from[0] == '-') &&
           (std::isdigit((unsigned char)to[0])   || to[0]   == '-');
}

static vector<string> expandNumericSequence(const string &from, const string &to, int stepOverride = 0)
{
    int start = stoi(from), end = stoi(to);
    bool ascending = (start <= end);
    int step = ascending ? 1 : -1;
    if (stepOverride != 0)
        step = ascending ? std::abs(stepOverride) : -std::abs(stepOverride);

    bool zeroPad = (from.size() > 1 && from[0] == '0') ||
                   (to.size()   > 1 && to[0]   == '0');
    int width = zeroPad ? (int)std::max(from.size(), to.size()) : 0;

    vector<string> result;
    if (ascending)
    {
        for (int n = start; n <= end; n += step)
        {
            string s = to_string(n);
            if (zeroPad) while ((int)s.size() < width) s = "0" + s;
            result.push_back(s);
        }
    }
    else
    {
        for (int n = start; n >= end; n += step)
        {
            string s = to_string(n);
            if (zeroPad) while ((int)s.size() < width) s = "0" + s;
            result.push_back(s);
        }
    }
    return result;
}

static vector<string> expandLetterSequence(char from, char to, int stepOverride = 0)
{
    bool ascending = (from <= to);
    int step = ascending ? 1 : -1;
    if (stepOverride != 0)
        step = ascending ? std::abs(stepOverride) : -std::abs(stepOverride);

    vector<string> result;
    if (ascending)
    {
        for (int c = (int)from; c <= (int)to; c += step)
            result.push_back(string(1, (char)c));
    }
    else
    {
        for (int c = (int)from; c >= (int)to; c += step)
            result.push_back(string(1, (char)c));
    }
    return result;
}

// Turns "from..to[..step]" into its list of values. Empty vector = invalid sequence.
static vector<string> expandSequence(const string &from, const string &toAndStep)
{
    // Detect optional "..step" suffix inside the `to` argument.
    // e.g. solvePattern("1..10..2") splits left="1", right="10..2"
    // so toAndStep = "10..2" here.
    string to       = toAndStep;
    int    step     = 0;
    size_t stepDot  = toAndStep.find("..");
    if (stepDot != string::npos)
    {
        string stepStr = toAndStep.substr(stepDot + 2);
        to = toAndStep.substr(0, stepDot);
        try { step = stoi(stepStr); }
        catch (...) { return {}; } // unparseable step → invalid
        if (step == 0) return {};  // zero step → infinite loop guard
    }

    if (looksNumeric(from, to))
        return expandNumericSequence(from, to, step);
    if (from.size() == 1 && to.size() == 1)
        return expandLetterSequence(from[0], to[0], step);
    return {};
}

// Grammar solver: Pattern → Pattern Operator Pattern | atom

// Recursively solves the Pattern-Operator-Pattern grammar over `s`.
// - No top-level operator  → atom, return { s } unchanged.
// - Operator is ','        → return solve(left) followed by solve(right).
// - Operator is '..'       → left/right are sequence endpoints; expand them.
//                            Invalid sequence → keep "left..right" as literal.
static vector<string> solvePattern(const string &s)
{
    OperatorSplit op = findTopLevelOperator(s);
    if (!op.found)
        return { s }; // atom — nothing to expand at this level

    string left  = s.substr(0, op.pos);
    string right = s.substr(op.pos + op.len);

    if (op.type == ',')
    {
        vector<string> result = solvePattern(left);
        vector<string> rightResult = solvePattern(right);
        result.insert(result.end(), rightResult.begin(), rightResult.end());
        return result;
    }

    // op.type == '.' → ".." sequence
    vector<string> seq = expandSequence(left, right);
    if (seq.empty())
        return { left + ".." + right }; // invalid sequence → literal
    return seq;
}

// Public entry point: locate {...}, solve inside, recurse

vector<string> expandBraces(const string &pattern)
{
    size_t openPos = pattern.find('{');
    if (openPos == string::npos)
        return { pattern }; // no braces at all

    size_t closePos = findMatchingBrace(pattern, openPos);
    if (closePos == string::npos)
        return { pattern }; // unmatched '{' → literal

    string prefix = pattern.substr(0, openPos);
    string inside = pattern.substr(openPos + 1, closePos - openPos - 1);
    string suffix = pattern.substr(closePos + 1);

    vector<string> parts = solvePattern(inside);

    // No operator was found at all → this brace group isn't a real
    // expansion (e.g. "{nocomma}") → keep the whole original literal.
    if (parts.size() == 1 && parts[0] == inside)
        return { pattern };

    vector<string> result;
    for (const string &part : parts)
        for (const string &expanded : expandBraces(prefix + part + suffix))
            result.push_back(expanded);
    return result;
}