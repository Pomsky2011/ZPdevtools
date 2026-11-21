#!/usr/bin/env bash
# The C layers library end to end: builds the game template (game-template/, art from PNGs through png2zp) with each C compiler, runs the ROM headless on the emulator, dumps frames and
# checks every pixel against check_frames.py's reconstruction. The 60 Hz pacing is checked by the same thing: each frame must show the game
# step it should, with a gap that never changes. Runs twice per compiler: on the native boot, and through the real Boot ROM (when
# ../ZPbootROM/build/bootrom.bin or $BOOT exists), which hands off in a different machine state (DB = $80, a 512-byte stack) and needs the ROM
# signed the way the console's hash engine checks it. Then once per compiler on the native boot: the same code with other art
# (swap_assets.py: a BG0 of one tile placed mirrored four ways) must draw that art, which also proves the flip bits.
#
#   tests/layers/run.sh [zpcc|dos ...]        (default: both)        TEMPLATE=dir to build another checkout of the template
# Needs: ZeroPoint's build/bin/dump_splash (ZP_BUILD), the toolchain built (make in ZPdevtools, llvm_compiler, c_compiler), python3.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ZP_BUILD=${ZP_BUILD:-$ROOT/../ZeroPoint/build}
DUMP=$ZP_BUILD/bin/dump_splash
BOOT=${BOOT:-$ROOT/../ZPbootROM/build/bootrom.bin}
TEMPLATE=${TEMPLATE:-$ROOT/game-template}
WORK=${WORK:-$(mktemp -d)}
ccs=("$@"); [ ${#ccs[@]} -eq 0 ] && ccs=(zpcc dos)
[ -x "$DUMP" ] || { echo "missing $DUMP (build ZeroPoint)" >&2; exit 2; }
fail=0
for cc in "${ccs[@]}"; do
  b=$WORK/$cc
  if ! make -s -C "$TEMPLATE" ZPDEV="$ROOT" CC=$cc BUILD="$b" > "$WORK/$cc.make.log" 2>&1; then
    echo "  FAIL: the game template does not build with $cc (see $WORK/$cc.make.log)"; fail=1; continue
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
  # other art, same code
  sw=$WORK/swap
  python3 "$HERE/swap_assets.py" gen "$sw" "$TEMPLATE/assets" && \
  if ! make -s -C "$TEMPLATE" ZPDEV="$ROOT" CC=$cc BUILD="$b-swap" ASSETS="$sw" > "$WORK/$cc.swap.make.log" 2>&1; then
    echo "  FAIL: the template does not build with swapped art on $cc (see $WORK/$cc.swap.make.log)"; fail=1; continue
  fi
  rm -f "$b-swap"/native_*.rgba
  "$DUMP" none "$b-swap/game.zpb" "$b-swap/native" 30 100 150 > "$WORK/$cc.swap.dump.log" 2>&1
  echo "== $cc, swapped art (mirrored tiles)"
  python3 "$HERE/swap_assets.py" check "$sw" "$b-swap" "$b-swap/native_30.rgba" "$b-swap/native_100.rgba" "$b-swap/native_150.rgba" || fail=1
done
[ $fail -eq 0 ] && echo "all ok"
exit $fail
