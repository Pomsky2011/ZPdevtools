#!/bin/sh
# cpuasm/apuasm/ppuasm must refuse input that does not fit (over-long lines and names, more than 64 KiB of code) with an
# error and exit 1 (as must apuasm for an operand that is neither a number nor a label), never overflow a buffer or silently wrap, and must still take the largest input that does fit.
# Builds ASan/UBSan copies of the three assemblers, so a crash is a failure here even if it would "work" unsanitised.
#   tests/asm/robust.sh
HERE=$(cd "$(dirname "$0")" && pwd)
SRC=$HERE/../../src
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
for t in cpuasm apuasm ppuasm; do
    gcc -std=c89 -O1 -fsanitize=address,undefined -fno-sanitize-recover=undefined -I"$SRC" "$SRC/$t.c" -o "$W/$t" || exit 1
done
bad=0
gen() { python3 -c "import sys; sys.stdout.write($1)" > "$W/in.asm"; }
# expect (ok|err) tool description  (source is in $W/in.asm); "err" also fails on a sanitizer report or a signal
try() {
    "$W/$2" "$W/in.asm" "$W/out.bin" >"$W/log" 2>&1; rc=$?
    if grep -q 'Sanitizer\|runtime error' "$W/log" || [ $rc -ge 128 ]; then echo "FAIL: $2 $3: crashed"; bad=$((bad+1));
    elif [ "$1" = ok ] && [ $rc -ne 0 ]; then echo "FAIL: $2 $3: should assemble, exit $rc"; bad=$((bad+1));
    elif [ "$1" = err ] && [ $rc -ne 1 ]; then echo "FAIL: $2 $3: should be refused with exit 1, exit $rc"; bad=$((bad+1)); fi
}
gen "'.org \$0200\n.bank \$80\nLDA (' + 'A'*400 + ')\n'";            try err cpuasm "400-char operand"
gen "'.org \$0200\nNOP ;' + 'c'*300 + '\n'";                          try err cpuasm "300-char line"
gen "'.org \$0200\n' + 'L'*80 + ': NOP\n'";                           try err cpuasm "80-char label"
gen "'.EQU ' + 'N'*100 + ' 5\n'";                                     try err cpuasm "100-char .EQU name"
gen "'BNE ' + 'x'*240 + '\n'";                                        try err cpuasm "BNE with a long target"
gen "'.include \"\n'";                                                try err cpuasm ".include with an empty name"
gen "'.org \$0000\n' + 'NOP\n'*65537";                                try err cpuasm "65537 bytes of code"
gen "'.org \$0200\n' + 'NOP\n'*65400";                                try err cpuasm "code past \$FFFF from .org \$0200"
gen "'.org \$0000\n' + 'NOP\n'*65536";                                try ok  cpuasm "exactly 65536 bytes of code"
[ "$(wc -c < "$W/out.bin")" -eq 65536 ] || { echo "FAIL: cpuasm: 65536 NOPs wrote $(wc -c < "$W/out.bin") bytes"; bad=$((bad+1)); }
gen "'NOP ;' + 'c'*250 + '\n'";                                        try ok  cpuasm "255-char line"
gen "'LDA ' + 'A'*600 + '\n'";                                        try err apuasm "600-char line"
gen "'L'*70 + ': NOP\n'";                                             try err apuasm "70-char label"
gen "'NOP\n'";                                                        try ok  apuasm "NOP"
gen "'LDA zzzz_not_a_label\n'";                                      try err apuasm "undefined label (used to assemble as 0)"
gen "'start:\n JMP fwd\n NOP\nfwd:\n NOP\n'";                       try ok  apuasm "forward reference to a later label"
gen "'L'*300 + ': NOP\n'";                                            try err ppuasm "300-char line"
gen "'NOP ' + 'A'*100 + '\n'";                                        try err ppuasm "100-char operand"
gen "'L'*80 + ': NOP\n'";                                             try err ppuasm "80-char label"
gen "'a = ' + 'b'*100 + '\n'";                                        try err ppuasm "100-char alias"
gen "'start:\n JMR start\n HLT\n'";                                   try ok  ppuasm "label, JMR, HLT"
[ $bad -eq 0 ] && echo "all assembler robustness checks passed"
[ $bad -eq 0 ]
