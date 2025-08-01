#!/bin/sh -e

cd "$(dirname "$0")"

tmpd=$(mktemp -d)
trap 'rm -rf "$tmpd"' EXIT

DIFF_COLOR=never
C_RED=
C_GREEN=
C_BLUE=
C_RESET=
if [ -t 1 ]; then
    DIFF_COLOR=always
    C_RED=$(tput setaf 1)
    C_GREEN=$(tput setaf 2)
    C_BLUE=$(tput setaf 4)
    C_RESET=$(tput sgr0)
fi

failed=0

for f in test_*; do
    if [ "$TEST_PATTERN" ] && ! echo "$f" | grep -qP "$TEST_PATTERN"; then
        continue
    fi
    test_sequence=$(sed  -n '1p;q' "$f")
    ../build/ant --test "$test_sequence" > "$tmpd/$f.actual"
    sed  '1d' "$f" > "$tmpd/$f.expected"
    delta=$(diff -u --color=$DIFF_COLOR "$tmpd/$f.expected" "$tmpd/$f.actual"|sed -n '1b;2b;s/^/  /p' || true)
    if [ "$TEST_SAVE" ] && [ "$delta" ]; then
        echo "$C_BLUE- Saving test $f snapshot:$C_RESET"
        echo "$delta"
        echo ""
        echo "$test_sequence" > "$f"
        cat "$tmpd/$f.actual" >> "$f"
    elif [ "$delta" ]; then
        echo "$C_RED- Test $f FAIL:$C_RESET"
        echo "$delta"
        echo ""
        failed=$((failed+1))
    fi
done

if [ $failed -eq 0 ]; then
    echo "$C_GREEN+ All tests passed$C_RESET"
fi
