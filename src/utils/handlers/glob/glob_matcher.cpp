#include <string>
#include <cctype>
#include <unordered_map>
using std::string;

// POSIX named class lookup: [:alpha:], [:digit:], etc.
static bool matchPosixClass(char c, const string &className)
{
    static const std::unordered_map<std::string, int(*)(int)> classMap = {
        {"alpha",  std::isalpha},   // A-Z, a-z
        {"digit",  std::isdigit},   // 0-9
        {"alnum",  std::isalnum},   // A-Z, a-z, 0-9
        {"upper",  std::isupper},   // A-Z
        {"lower",  std::islower},   // a-z
        {"space",  std::isspace},   // ' ' \t \n \v \f \r
        {"punct",  std::ispunct},   // punctuation/symbol chars
        {"print",  std::isprint},   // printable incl. space
        {"graph",  std::isgraph},   // printable excl. space
        {"xdigit", std::isxdigit},  // 0-9 A-F a-f
        {"cntrl",  std::iscntrl},   // control chars
    };

    if (className == "blank")
        return c == ' ' || c == '\t';

    auto it = classMap.find(className);
    if (it != classMap.end())
        return it->second((unsigned char)c) != 0; // cast avoids UB on negative char

    return false; // unknown class name → no match
}

// Bracket-expression helpers

// Consumes leading '^' or '!' negation marker; returns true if present.
static bool consumeNegation(const string &pat, size_t &patPos)
{
    if (patPos < pat.size() && (pat[patPos] == '^' || pat[patPos] == '!'))
    {
        ++patPos;
        return true;
    }
    return false;
}

// Parses one "[:name:]" token and evaluates it against nameChar.
static bool matchPosixClassToken(const string &pat, size_t &patPos, char nameChar)
{
    patPos += 2; // skip '[:'
    size_t classNameStart = patPos;
    while (patPos < pat.size() &&
           !(pat[patPos] == ':' && patPos + 1 < pat.size() && pat[patPos + 1] == ']'))
        ++patPos;
    string className = pat.substr(classNameStart, patPos - classNameStart);
    patPos += 2; // skip ':]'
    return matchPosixClass(nameChar, className);
}

// Parses one range (a-z) or literal char and evaluates it against nameChar.
static bool matchRangeOrLiteral(const string &pat, size_t &patPos, char nameChar)
{
    char classChar = pat[patPos++];
    if (patPos + 1 < pat.size() && pat[patPos] == '-' && pat[patPos + 1] != ']')
    {
        ++patPos; // skip '-'
        char rangeEnd = pat[patPos++];
        return nameChar >= classChar && nameChar <= rangeEnd;
    }
    return nameChar == classChar; // literal
}

// Parses a full [...] bracket expression (patPos just past '[') against nameChar.
static bool matchCharClass(const string &pat, size_t &patPos, char nameChar)
{
    bool negated = consumeNegation(pat, patPos);
    bool classMatched = false;
    bool isFirstCharInClass = true; // lets ']' be literal as first char, e.g. []abc]

    while (patPos < pat.size() && (isFirstCharInClass || pat[patPos] != ']'))
    {
        isFirstCharInClass = false;

        bool elementMatched;
        if (pat[patPos] == '[' && patPos + 1 < pat.size() && pat[patPos + 1] == ':')
            elementMatched = matchPosixClassToken(pat, patPos, nameChar);
        else
            elementMatched = matchRangeOrLiteral(pat, patPos, nameChar);

        if (elementMatched)
            classMatched = true; // keep scanning to reach closing ']'
    }

    if (patPos < pat.size()) ++patPos; // skip closing ']'

    return (classMatched && !negated) || (!classMatched && negated);
}

// Top-level glob matcher
bool matchGlob(const string &pattern, const string &name)
{
    size_t patPos = 0, namePos = 0;
    size_t lastStarPatPos = string::npos, lastStarNamePos = 0;

    while (namePos < name.size())
    {
        if (patPos < pattern.size() && pattern[patPos] == '[')
        {
            ++patPos; // skip opening '['
            bool accepted = matchCharClass(pattern, patPos, name[namePos]);
            if (accepted)
            {
                ++namePos;
            }
            else if (lastStarPatPos != string::npos)
            {
                patPos  = lastStarPatPos + 1; // backtrack to last '*'
                namePos = ++lastStarNamePos;
            }
            else
            {
                return false;
            }
        }
        else if (patPos < pattern.size() && pattern[patPos] == '*')
        {
            lastStarPatPos  = patPos++; // bookmark, then optimistically skip '*'
            lastStarNamePos = namePos;
        }
        else if (patPos < pattern.size() &&
                 (pattern[patPos] == name[namePos] || pattern[patPos] == '?'))
        {
            ++patPos; // literal match or '?' wildcard
            ++namePos;
        }
        else if (lastStarPatPos != string::npos)
        {
            patPos  = lastStarPatPos + 1; // let last '*' absorb one more char
            namePos = ++lastStarNamePos;
        }
        else
        {
            return false; // no match, no star to fall back on
        }
    }

    while (patPos < pattern.size() && pattern[patPos] == '*')
        ++patPos; // trailing '*' matches empty string

    return patPos == pattern.size(); // true only if pattern also fully consumed
}