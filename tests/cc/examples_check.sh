#!/bin/sh
# c_compiler/examples/*.c must all compile; c_compiler/examples/unsupported/*.c use constructs def88186cc refuses and
# must all fail cleanly (non-zero exit, no output file, no crash).
#   tests/cc/examples_check.sh [path/to/def88186cc]
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
DCC=${1:-${DEF88186CC:-$ROOT/c_compiler/def88186cc}}
W=$(mktemp -d)
bad=0; n=0
for f in "$ROOT"/c_compiler/examples/*.c; do
    case $f in *multifile*) continue;; esac       # needs its sibling files, built by the -bin flow
    n=$((n+1)); b=$(basename "$f" .c)
    if ! "$DCC" -I "$ROOT/include" -I "$ROOT/c_compiler/examples" "$f" -o "$W/$b.asm" >"$W/$b.log" 2>&1; then
        echo "FAIL (should compile): $b"; tail -2 "$W/$b.log"; bad=$((bad+1))
    fi
done
for f in "$ROOT"/c_compiler/examples/unsupported/*.c; do
    n=$((n+1)); b=$(basename "$f" .c)
    "$DCC" -I "$ROOT/include" "$f" -o "$W/u_$b.asm" >"$W/u_$b.log" 2>&1; rc=$?
    if [ $rc -eq 0 ] || [ $rc -ge 128 ] || [ -f "$W/u_$b.asm" ]; then
        echo "FAIL (should be refused cleanly, rc=$rc): $b"; bad=$((bad+1))
    fi
done
rm -rf "$W"
echo "$((n-bad)) of $n example checks passed"
[ $bad -eq 0 ]
