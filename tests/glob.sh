#!/usr/bin/env bash
# glob.sh — tests for glob expansion (*, ?, [...], **, brace expansion)
# Usage: bash tests/glob.sh <path-to-binary>
# Exit code: 0 = all passed, non-zero = failures

set -euo pipefail

BIN="${1:?Usage: glob.sh <path-to-binary>}"

if [[ ! -x "$BIN" ]]; then
    echo "ERROR: binary not found or not executable: $BIN"
    exit 1
fi

PASS=0
FAIL=0

# ── Temp filesystem fixture ───────────────────────────────────────────────────
FIXTURE="glob_fixture_$$"
mkdir -p "$FIXTURE/subdir/nested"

touch "$FIXTURE/alpha.txt"
touch "$FIXTURE/beta.txt"
touch "$FIXTURE/gamma.md"
touch "$FIXTURE/.hidden.txt"
touch "$FIXTURE/subdir/delta.txt"
touch "$FIXTURE/subdir/epsilon.txt"
touch "$FIXTURE/subdir/nested/zeta.txt"

trap 'rm -rf "$FIXTURE"' EXIT

ABS_FIXTURE="$(pwd)/$FIXTURE"
# Absolute path to binary so it still works after we cd into the fixture
BIN_ABS="$(cd "$(dirname "$BIN")" && pwd)/$(basename "$BIN")"

# ── Helpers ───────────────────────────────────────────────────────────────────
run() {
    local desc="$1" input="$2" expected="$3"
    local actual
    # Launch the shell with its cwd set to the fixture so relative globs work.
    actual="$(printf '%s\nexit\n' "$input" \
              | (cd "$ABS_FIXTURE" && "$BIN_ABS") 2>/dev/null | tr -d '\r')"
    if [[ "$actual" == *"$expected"* ]]; then
        echo "  PASS  $desc"
        PASS=$((PASS + 1))
    else
        echo "  FAIL  $desc"
        echo "        expected to contain: $(printf '%q' "$expected")"
        echo "        got:                 $(printf '%q' "$actual")"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== Glob Expansion Tests: $BIN ==="

# ── * wildcard ────────────────────────────────────────────────────────────────
run "* matches .txt files"        "echo *.txt"       "alpha.txt beta.txt"
run "* matches .md files"         "echo *.md"        "gamma.md"
run "* does not match hidden"     "echo *.txt"       "alpha.txt"   # .hidden.txt must NOT appear
run "* no match keeps literal"    "echo *.xyz"       "*.xyz"

# ── ? wildcard ────────────────────────────────────────────────────────────────
run "? matches single char"       "echo ?eta.txt"    "beta.txt"
run "? in middle"                 "echo g?mma.md"    "gamma.md"

# ── [...] character class ─────────────────────────────────────────────────────
run "[ab]* matches a/b prefix"    "echo [ab]*.txt"   "alpha.txt beta.txt"
run "[a-g]* range match"          "echo [a-g]amma.md" "gamma.md"
run "[^g]* negated class"         "echo [^g]*.txt"   "alpha.txt beta.txt"

# ── multi-segment (dir/*.txt) ─────────────────────────────────────────────────
run "dir/*.txt stays in subdir"   "echo subdir/*.txt"  "subdir/delta.txt subdir/epsilon.txt"
run "dir/* matches all in subdir" "echo subdir/*"      "subdir/delta.txt"

# ── ** recursive ──────────────────────────────────────────────────────────────
run "**/*.txt finds nested files" "echo **/*.txt"    "subdir/nested/zeta.txt"
run "subdir/**/*.txt recursive"   "echo subdir/**/*.txt" "subdir/nested/zeta.txt"

# ── hidden files ─────────────────────────────────────────────────────────────
run ".* matches hidden files"     "echo .*"          ".hidden.txt"

# ── brace expansion ───────────────────────────────────────────────────────────
run "{a,b} expands alternatives"  "echo {alpha,beta}.txt"   "alpha.txt beta.txt"
run "{a,b} prefix brace"          "echo {alpha,gamma}.*"    "alpha.txt gamma.md"
run "{1..3} numeric sequence"     "echo {1..3}"             "1 2 3"
run "{a..c} letter sequence"      "echo {a..c}"             "a b c"

# ── brace + glob combined ─────────────────────────────────────────────────────
run "brace prefix + glob suffix"  "echo subdir/{del,eps}*.txt" "subdir/delta.txt subdir/epsilon.txt"

# ── summary ──────────────────────────────────────────────────────────────────
echo ""
echo "Results: $PASS passed, $FAIL failed"
[[ "$FAIL" -eq 0 ]]
