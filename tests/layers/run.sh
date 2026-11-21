#!/usr/bin/env bash
# The C layers library end to end: builds examples/layers-game with each C compiler, runs the ROM headless on the emulator (native boot),
# dumps frames and checks every pixel against check_frames.py's reconstruction. The 60 Hz pacing is checked by the same thing: the frame
# numbers are rasters, and each must show the game step it should.
#
#   tests/layers/run.sh [zpcc|dos ...]        (default: both)
# Needs: ZeroPoint's build/bin/dump_splash (ZP_BUILD), the toolchain built (make in ZPdevtools, llvm_compiler, c_compiler), python3.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ZP_BUILD=${ZP_BUILD:-$ROOT/../ZeroPoint/build}
DUMP=$ZP_BUILD/bin/dump_splash
WORK=${WORK:-$(mktemp -d)}
FRAMES=${FRAMES:-"30 61 100 150"}
ccs=("$@"); [ ${#ccs[@]} -eq 0 ] && ccs=(zpcc dos)
[ -x "$DUMP" ] || { echo "missing $DUMP (build ZeroPoint)" >&2; exit 2; }
fail=0
for cc in "${ccs[@]}"; do
  b=$WORK/$cc
  if ! make -s -C "$ROOT/examples/layers-game" CC=$cc BUILD="$b" > "$WORK/$cc.make.log" 2>&1; then
    echo "  FAIL: layers-game does not build with $cc (see $WORK/$cc.make.log)"; fail=1; continue
  fi
  rm -f "$b"/f_*.rgba
  # shellcheck disable=SC2086
  timeout 600 "$DUMP" none "$b/game.zpb" "$b/f" $FRAMES > "$WORK/$cc.dump.log" 2>&1
  files=(); for n in $FRAMES; do files+=("$b/f_$n.rgba"); done
  echo "== $cc"
  python3 "$HERE/check_frames.py" "$b" "${files[@]}" || fail=1
done
[ $fail -eq 0 ] && echo "all ok"
exit $fail
