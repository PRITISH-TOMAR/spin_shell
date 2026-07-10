#include "parser.hpp"
#include <string>
#include <vector>
#include <cctype>
using std::string;
using std::vector;

// Advances `i` past any run of whitespace.
static void skipWhitespace(const string &input, size_t &i)
{
    while (i < input.size() && isspace((unsigned char)input[i]))
        ++i;
}

// True if `c` is a glob metacharacter that would trigger wildcard expansion
// if left unquoted.
static bool isGlobChar(char c)
{
    return c == '*' || c == '?' || c == '[' || c == '{';
}

// Reads a quoted segment starting at the quote char input[i] (either ' or ").
// Appends the content between the quotes (not the quotes themselves) to
// `token`, and advances `i` past the closing quote.
static void consumeQuotedSegment(const string &input, size_t &i, string &token)
{
    char quoteChar = input[i++]; // remember which quote, skip opening quote
    while (i < input.size() && input[i] != quoteChar)
        token += input[i++];
    if (i < input.size())
        ++i; // skip closing quote
}

// Result of extracting one whitespace-delimited token.
struct Token
{
    string text;
    bool   hasUnquotedGlob;
};

// Reads one token starting at `i` (assumes leading whitespace already
// skipped), handling quoted segments and tracking unquoted glob chars.
static Token consumeToken(const string &input, size_t &i)
{
    Token tok{ "", false };

    while (i < input.size() && !isspace((unsigned char)input[i]))
    {
        char c = input[i];
        if (c == '\'' || c == '"')
        {
            consumeQuotedSegment(input, i, tok.text);
        }
        else
        {
            if (isGlobChar(c))
                tok.hasUnquotedGlob = true;
            tok.text += c;
            ++i;
        }
    }
    return tok;
}


static bool isShortFlagBundle(const string &token)
{
    return token.size() > 1 && token[0] == '-' && token[1] != '-';
}

static bool isLongFlag(const string &token)
{
    return token.size() > 2 && token.substr(0, 2) == "--";
}

static void recordShortFlags(const string &token, ParsedInput &parsed)
{
    for (size_t j = 1; j < token.size(); ++j)
        parsed.shortFlags[token[j]] = true;
}

// Records the name after "--" as a long flag.
static void recordLongFlag(const string &token, ParsedInput &parsed)
{
    parsed.longFlags[token.substr(2)] = true;
}

static void classifyArgument(const Token &tok, ParsedInput &parsed)
{
    parsed.rawArgs.push_back(tok.text);
    parsed.rawArgsHasGlob.push_back(tok.hasUnquotedGlob);

    if (isShortFlagBundle(tok.text))
        recordShortFlags(tok.text, parsed);
    else if (isLongFlag(tok.text))
        recordLongFlag(tok.text, parsed);
    else
        parsed.files.push_back(tok.text);
}


// Parses a raw command-line string into command name, flags, files,
// and raw argument list (with per-argument unquoted-glob tracking).
ParsedInput parseInput(const string &input)
{
    ParsedInput parsed;
    size_t i = 0;
    bool first = true;

    while (i < input.size())
    {
        skipWhitespace(input, i);
        if (i >= input.size())
            break;

        Token tok = consumeToken(input, i);

        if (first)
        {
            parsed.command = tok.text;
            first = false;
            continue;
        }

        classifyArgument(tok, parsed);
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
