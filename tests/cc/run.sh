#!/usr/bin/env bash
# Compiler regression tests: build every tests/cc/t*.c with the host gcc (reference) and with zpcc and def88186cc, run the
# latter two in the emulator and compare what they recorded.
#
#   tests/cc/run.sh [-v] [zpcc|dos ...] [-- test.c ...]        (default: both compilers, every t*.c)
#
# A test with a line "/* ONLY: zpcc */" (or "dos") runs under that compiler only.
# A test with a line "/* EXPECT: v1 v2 ... */" is checked against that list instead of a host run (SDK tests can't run
# on the host). "/* NEEDS: boot */" tests need the real boot ROM: set BOOT=path/to/bootrom.bin or they are skipped.
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
  if grep -q 'NEEDS: boot' "$t" && [ -z "${BOOT:-}" ]; then echo "skip $name (needs BOOT=bootrom.bin)"; continue; fi
  only=$(sed -n 's|^/\* ONLY: \(.*\) \*/$|\1|p' "$t" | head -1)     # a test for one compiler only
  expect=$(sed -n 's|^/\* EXPECT: \(.*\) \*/$|\1|p' "$t" | head -1)
  if [ -n "$expect" ]; then
    want="$expect "
  else
  gcc -DHOST -w -I"$HERE" -o "$WORK/$name.host" "$t" 2>"$WORK/$name.host.err" || { echo "HOST BUILD FAILED: $name"; cat "$WORK/$name.host.err"; fail=$((fail+1)); continue; }
  want=$("$WORK/$name.host")
  fi
  for cc in "${ccs[@]}"; do
    if [ -n "$only" ] && ! echo " $only " | grep -q " $cc "; then continue; fi
    asm="$WORK/$name.$cc.asm"
    if [ "$cc" = zpcc ]; then
      "$ROOT/llvm_compiler/zpcc" -I "$ROOT/include" -I "$HERE" "$t" -o "$asm" >"$WORK/$name.$cc.log" 2>&1; crc=$?
    else
      "$ROOT/c_compiler/def88186cc" -DZP_CC_DOS -I "$ROOT/include" -I "$HERE" "$t" -o "$asm" >"$WORK/$name.$cc.log" 2>&1; crc=$?
    fi
    # a compiler that reported an error has failed, whatever file it left behind
    if [ $crc -ne 0 ] || [ ! -f "$asm" ]; then echo "FAIL $name [$cc]: compile error"; [ $verbose = 1 ] && tail -5 "$WORK/$name.$cc.log"; fail=$((fail+1)); continue; fi
    {
      echo ".org \$0200"; echo ".bank \$80"
      echo ".include \"$ROOT/lib/crt0_entry.asm\""
      echo ".include \"$asm\""
      echo ".include \"$HERE/rec.asm\""
      echo ".include \"$ROOT/lib/zp_mmio.asm\""
      echo ".include \"$ROOT/lib/zp_cop.asm\""
      [ -f "$ROOT/lib/zp_long.asm" ] && echo ".include \"$ROOT/lib/zp_long.asm\""
    } > "$WORK/$name.$cc.img.asm"
    if ! "$ROOT/executables/cpuasm" "$WORK/$name.$cc.img.asm" "$WORK/$name.$cc.bin" >"$WORK/$name.$cc.as.log" 2>&1; then
      echo "FAIL $name [$cc]: assemble error: $(grep -m1 -i error "$WORK/$name.$cc.as.log")"; fail=$((fail+1)); continue
    fi
    got=$("$RUN" "${BOOT:-none}" "$WORK/$name.$cc.bin" | tail -1)
    if [ "$got" = "$want" ]; then pass=$((pass+1)); echo "ok   $name [$cc]"
    else fail=$((fail+1)); echo "FAIL $name [$cc]"; echo "     want: $want"; echo "     got : $got"; fi
  done
done
echo "$pass passed, $fail failed"
[ $fail -eq 0 ]
