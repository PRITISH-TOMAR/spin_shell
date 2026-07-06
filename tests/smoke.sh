#!/usr/bin/env bash
# smoke.sh — basic sanity tests for the shell binary
# Usage: bash tests/smoke.sh <path-to-binary>
# Exit code: 0 = all passed, non-zero = failures

set -euo pipefail

BIN="${1:?Usage: smoke.sh <path-to-binary>}"

if [[ ! -x "$BIN" ]]; then
    echo "ERROR: binary not found or not executable: $BIN"
    exit 1
fi

PASS=0
FAIL=0

# Use a relative temp dir so paths work on Windows too
TMPDIR_TEST="smoke_tmp_$$"
mkdir -p "$TMPDIR_TEST"
trap 'rm -rf "$TMPDIR_TEST"' EXIT

run() {
    # run <description> <input> <expected-substring>
    local desc="$1" input="$2" expected="$3"
    local actual
    actual="$(printf '%s\nexit\n' "$input" | "$BIN" 2>/dev/null | tr -d '\r')"
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

run_exit() {
    # run_exit <description> <input> <expected-exit-code>
    local desc="$1" input="$2" expected_code="$3"
    local actual_code=0
    printf '%s\n' "$input" | "$BIN" > /dev/null 2>&1 || actual_code=$?
    if [[ "$actual_code" -eq "$expected_code" ]]; then
        echo "  PASS  $desc"
        PASS=$((PASS + 1))
    else
        echo "  FAIL  $desc"
        echo "        expected exit code $expected_code, got $actual_code"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== Smoke Tests: $BIN ==="

# ── echo ─────────────────────────────────────────────────────────────────────
run "echo hello"          "echo hello"      "hello"
run "echo multiple words" "echo foo bar baz" "foo bar baz"
run "echo empty"          "echo"            ""

# ── pwd ──────────────────────────────────────────────────────────────────────
# Just check it prints something — path format differs per OS
actual_pwd="$(printf 'pwd\nexit\n' | "$BIN" 2>/dev/null | tr -d '\r')"
if [[ -n "$actual_pwd" ]]; then
    echo "  PASS  pwd prints a path"
    PASS=$((PASS + 1))
else
    echo "  FAIL  pwd prints a path (empty output)"
    FAIL=$((FAIL + 1))
fi

# ── cd + pwd ─────────────────────────────────────────────────────────────────
# cd to parent then back; just verify pwd output changes / is non-empty
actual_cd="$(printf 'cd ..\npwd\nexit\n' | "$BIN" 2>/dev/null | tr -d '\r')"
if [[ -n "$actual_cd" ]]; then
    echo "  PASS  cd .. then pwd non-empty"
    PASS=$((PASS + 1))
else
    echo "  FAIL  cd .. then pwd non-empty"
    FAIL=$((FAIL + 1))
fi

# ── exit codes ───────────────────────────────────────────────────────────────
run_exit "exit 0"  "exit 0"  0
run_exit "exit 1"  "exit 1"  1
run_exit "exit 42" "exit 42" 42

# ── $? expansion ─────────────────────────────────────────────────────────────
# Run a successful command, then check $? is 0
run 'echo $? after successful command' 'echo hi
echo $?' "0"

# ── redirection ──────────────────────────────────────────────────────────────
REDIR_FILE="$TMPDIR_TEST/out.txt"
printf 'echo redirected > %s\nexit\n' "$REDIR_FILE" | "$BIN" > /dev/null 2>&1
if [[ -f "$REDIR_FILE" ]] && grep -q "redirected" "$REDIR_FILE"; then
    echo "  PASS  stdout redirect (> file)"
    PASS=$((PASS + 1))
else
    echo "  FAIL  stdout redirect (> file)"
    FAIL=$((FAIL + 1))
fi

# ── cat ──────────────────────────────────────────────────────────────────────
CATFILE="$TMPDIR_TEST/cat_test.txt"
printf 'hello from cat\n' > "$CATFILE"
run "cat a file" "cat $CATFILE" "hello from cat"

# ── unknown command doesn't crash ────────────────────────────────────────────
run_exit "unknown command exits non-zero" "definitelynotacommand_xyz" 127

# ── summary ──────────────────────────────────────────────────────────────────
echo ""
echo "Results: $PASS passed, $FAIL failed"
[[ "$FAIL" -eq 0 ]]
