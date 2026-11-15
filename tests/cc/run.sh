#!/usr/bin/env bash
# Compiler regression tests: build every tests/cc/t*.c with the host gcc (reference) and with zpcc and def88186cc, run the
# latter two in the emulator and compare what they recorded.
#
#   tests/cc/run.sh [-v] [zpcc|dos ...] [-- test.c ...]        (default: both compilers, every t*.c)
#
# Needs: the emulator's zp_run_c (ZP_BUILD, default ../../../ZeroPoint/build), zpcc (make in llvm_compiler),
# def88186cc (make in c_compiler), cpuasm, gcc.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ZP_BUILD=${ZP_BUILD:-$ROOT/../ZeroPoint/build}
RUN=$ZP_BUILD/bin/zp_run_c
WORK=${WORK:-$(mktemp -d)}
verbose=0; ccs=(); tests=()
while [ $# -gt 0 ]; do
  case $1 in
    -v) verbose=1 ;;
    zpcc|dos) ccs+=("$1") ;;
    --) shift; tests=("$@"); break ;;
    *) echo "unknown argument $1" >&2; exit 2 ;;
  esac; shift
done
[ ${#ccs[@]} -eq 0 ] && ccs=(zpcc dos)
[ ${#tests[@]} -eq 0 ] && tests=("$HERE"/t*.c)
[ -x "$RUN" ] || { echo "missing $RUN (build ZeroPoint's zp_run_c)" >&2; exit 2; }

pass=0; fail=0
for t in "${tests[@]}"; do
  name=$(basename "$t" .c)
  gcc -DHOST -w -I"$HERE" -o "$WORK/$name.host" "$t" 2>"$WORK/$name.host.err" || { echo "HOST BUILD FAILED: $name"; cat "$WORK/$name.host.err"; fail=$((fail+1)); continue; }
  want=$("$WORK/$name.host")
  for cc in "${ccs[@]}"; do
    asm="$WORK/$name.$cc.asm"
    if [ "$cc" = zpcc ]; then
      "$ROOT/llvm_compiler/zpcc" -I "$ROOT/include" -I "$HERE" "$t" -o "$asm" >"$WORK/$name.$cc.log" 2>&1
    else
      "$ROOT/c_compiler/def88186cc" -DZP_CC_DOS -I "$ROOT/include" -I "$HERE" "$t" -o "$asm" >"$WORK/$name.$cc.log" 2>&1
    fi
    if [ ! -f "$asm" ]; then echo "FAIL $name [$cc]: compile error"; [ $verbose = 1 ] && tail -5 "$WORK/$name.$cc.log"; fail=$((fail+1)); continue; fi
    {
      echo ".org \$0200"; echo ".bank \$80"
      echo ".include \"$ROOT/lib/crt0_entry.asm\""
      echo ".include \"$asm\""
      echo ".include \"$HERE/rec.asm\""
      echo ".include \"$ROOT/lib/zp_mmio.asm\""
      [ -f "$ROOT/lib/zp_long.asm" ] && echo ".include \"$ROOT/lib/zp_long.asm\""
    } > "$WORK/$name.$cc.img.asm"
    if ! "$ROOT/executables/cpuasm" "$WORK/$name.$cc.img.asm" "$WORK/$name.$cc.bin" >"$WORK/$name.$cc.as.log" 2>&1; then
      echo "FAIL $name [$cc]: assemble error: $(grep -m1 -i error "$WORK/$name.$cc.as.log")"; fail=$((fail+1)); continue
    fi
    got=$("$RUN" none "$WORK/$name.$cc.bin" | tail -1)
    if [ "$got" = "$want" ]; then pass=$((pass+1)); echo "ok   $name [$cc]"
    else fail=$((fail+1)); echo "FAIL $name [$cc]"; echo "     want: $want"; echo "     got : $got"; fi
  done
done
echo "$pass passed, $fail failed"
[ $fail -eq 0 ]
