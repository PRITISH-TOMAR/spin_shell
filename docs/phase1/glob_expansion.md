# Glob Expansion — `src/utils/handlers/glob/`

## Overview

Two independent entry points compose together (in `input_handler.cpp`) to handle shell glob expansion:

1. **`expandBraces`** — brace expansion, no filesystem access
2. **`pathGlob`** — filesystem pattern matching
3. **`matchGlob`** — pure string pattern matcher (leaf, called by `pathGlob`)

---

## Composition

```
input_handler
    └── expandBraces(token)         <- first: expand {a,b} into N patterns
        └── for each pattern:
            pathGlob(pattern, cwd)  <- second: match each against filesystem
                walk(...)
                    matchGlob(...)  <- leaf: pure string matching
```

---

## 1. `expandBraces(pattern)` — `brace_expander.cpp`

**Purpose:** Expands `{a,b}` / `{1..5}` syntax into a list of strings. No filesystem access.

```
expandBraces(pattern)
├── findMatchingBrace(s, openPos)          — finds closing '}' tracking nesting depth
├── solvePattern(inside)                   — recursively parses the content of { }
│   ├── findTopLevelOperator(s)            — finds first ',' or '..' at depth 0
│   ├── solvePattern(left) [recursive]     — handles left side of operator
│   ├── solvePattern(right) [recursive]    — handles right side
│   └── expandSequence(from, toAndStep)    — handles '..' range operator
│       ├── looksNumeric(from, to)
│       ├── expandNumericSequence(...)     — generates 1 2 3 ... with optional step/zero-pad
│       └── expandLetterSequence(...)      — generates a b c ...
└── expandBraces(prefix + part + suffix)  — recurse on remaining braces in suffix
```

**Examples:**
- `file{.txt,.md}` → `["file.txt", "file.md"]`
- `{a,{b,c}}` → `["a", "b", "c"]`
- `{1..3}` → `["1", "2", "3"]`
- `no_braces` → `["no_braces"]`

---

## 2. `pathGlob(pattern, cwd)` — `path_glob.cpp`

**Purpose:** Expands a single glob pattern (no braces) against the real filesystem. Returns sorted matching paths relative to `cwd`.

```
pathGlob(pattern, cwd)
├── splitSegments(pattern)                 — splits "a/b/*.cpp" → ["a", "b", "*.cpp"]
└── walk(segs, segIdx, fsDir, relPrefix, results)   — recursive filesystem walker
    ├── matchGlob(seg, name)               — tests one segment vs one filename
    └── walk(segs, segIdx±1, ...)          — recurse: next segment or same (for **)
```

**`**` segment behavior:**
- As last segment: collects every entry at any depth recursively.
- Otherwise: tries next segment at current dir (zero levels), then descends into every subdir (one or more levels).

**Example:** `src/**/*.cpp` → iterates filesystem, returns all `.cpp` files under `src/`.

---

## 3. `matchGlob(pattern, name)` — `glob_matcher.cpp`

**Purpose:** Pure pattern-vs-string matcher. No filesystem. Called by `walk()` for each directory entry.

```
matchGlob(pattern, name)                   — iterative backtracking loop
├── matchCharClass(pat, patPos, nameChar)  — handles [...] bracket expressions
│   ├── consumeNegation(pat, patPos)       — strips leading '^' or '!'
│   ├── matchPosixClassToken(...)          — handles [[:alpha:]] etc.
│   └── matchRangeOrLiteral(...)           — handles [a-z] or literal [x]
└── matchPosixClass(c, className)          — maps class name → isalpha/isdigit/etc.
```

**Pattern rules:**
| Token | Meaning |
|---|---|
| `*` | Any chars except `/` |
| `?` | Exactly one non-`/` char |
| `[abc]` | Character class — literal, range, or POSIX named class |
| `[^abc]` / `[!abc]` | Negated character class |
| `[a-z]` | Character range |
| `[[:alpha:]]` | POSIX named class (`alpha`, `digit`, `alnum`, `upper`, `lower`, `space`, `punct`, `print`, `graph`, `xdigit`, `cntrl`, `blank`) |
| `**` | Handled by `walk()` — matches zero or more directory levels |

**Algorithm:** Iterative with backtracking. Bookmarks the last `*` position; on mismatch, backtracks and lets `*` absorb one more character.
