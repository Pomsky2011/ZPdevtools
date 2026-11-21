#!/usr/bin/env bash
# The C layers library end to end: builds examples/layers-game with each C compiler, runs the ROM headless on the emulator, dumps frames and
# checks every pixel against check_frames.py's reconstruction. The 60 Hz pacing is checked by the same thing: each frame must show the game
# step it should, with a gap that never changes. Runs twice per compiler: on the native boot, and through the real Boot ROM (when
# ../ZPbootROM/build/bootrom.bin or $BOOT exists), which hands off in a different machine state (DB = $80, a 512-byte stack) and needs the ROM
# signed the way the console's hash engine checks it.
#
#   tests/layers/run.sh [zpcc|dos ...]        (default: both)
# Needs: ZeroPoint's build/bin/dump_splash (ZP_BUILD), the toolchain built (make in ZPdevtools, llvm_compiler, c_compiler), python3.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ZP_BUILD=${ZP_BUILD:-$ROOT/../ZeroPoint/build}
DUMP=$ZP_BUILD/bin/dump_splash
BOOT=${BOOT:-$ROOT/../ZPbootROM/build/bootrom.bin}
WORK=${WORK:-$(mktemp -d)}
ccs=("$@"); [ ${#ccs[@]} -eq 0 ] && ccs=(zpcc dos)
[ -x "$DUMP" ] || { echo "missing $DUMP (build ZeroPoint)" >&2; exit 2; }
fail=0
for cc in "${ccs[@]}"; do
  b=$WORK/$cc
  if ! make -s -C "$ROOT/examples/layers-game" CC=$cc BUILD="$b" > "$WORK/$cc.make.log" 2>&1; then
    echo "  FAIL: layers-game does not build with $cc (see $WORK/$cc.make.log)"; fail=1; continue
  fi
  for mode in native real; do
    if [ $mode = native ]; then boot=none; frames="30 61 100 150"; else boot=$BOOT; frames="420 480 540"; fi
    if [ $mode = real ] && [ ! -f "$BOOT" ]; then echo "== $cc, real Boot ROM: SKIP (no $BOOT)"; continue; fi
    rm -f "$b/$mode"_*.rgba
    # shellcheck disable=SC2086
    timeout 900 "$DUMP" "$boot" "$b/game.zpb" "$b/$mode" $frames > "$WORK/$cc.$mode.dump.log" 2>&1
    files=(); for n in $frames; do files+=("$b/${mode}_$n.rgba"); done
    echo "== $cc, $([ $mode = native ] && echo native boot || echo real Boot ROM)"
    python3 "$HERE/check_frames.py" "$b" "${files[@]}" || fail=1
  done
done
[ $fail -eq 0 ] && echo "all ok"
exit $fail
