#!/bin/sh
# cpuasm must reject an operand that does not fit its field (LDA 256,S used to assemble silently as LDA 0,S), and must
# keep assembling everything that does fit, including the '>' force-long form for a label.
#   tests/asm/check.sh [path/to/cpuasm]
HERE=$(cd "$(dirname "$0")" && pwd)
ASM=${1:-$HERE/../../executables/cpuasm}
W=$(mktemp -d)
bad=0
try() {      # expect (ok|err) "source line" [expected bytes in hex]
    printf '.org $0200\n.bank $80\n%s\n' "$2" > "$W/t.asm"
    if "$ASM" "$W/t.asm" "$W/t.bin" >"$W/log" 2>&1; then got=ok; else got=err; fi
    bytes=
    [ $got = ok ] && bytes=$(od -An -tx1 -v "$W/t.bin" | tr -d ' \n')
    if [ "$got" != "$1" ]; then echo "FAIL: '$2' should be $1, was $got"; bad=$((bad+1));
    elif [ -n "$3" ] && [ "$bytes" != "$3" ]; then echo "FAIL: '$2' assembled to $bytes, expected $3"; bad=$((bad+1)); fi
}
try ok  'LDA 255,S'          '3fff'
try err 'LDA 256,S'
try err 'STA 70000,S'
try ok  'SEP #$20'           '1a20'
try err 'SEP #$200'
try ok  'LDA #$1234'         '373412'
try err 'LDA #$12345'
try ok  'LDA #-1'            '37ffff'
try ok  'LDA $123456,X'      '3e563412'
try ok  'LDA $FF'            '3bff'
try ok  'tbl: LDA >tbl'      '3d000280'
try ok  'tbl: LDA >tbl,X'    '3e000280'
try ok  'tbl: STA >tbl,X'    '56000280'
# .org: the first one sets the load address, a later one pads with zeros up to its address, and may not go backwards
# (it used to restart the output at byte 0, so a reset vector at .org $FFFC overwrote the start of the code)
org() {      # expect (ok|err) "source" [expected bytes in hex]
    printf '%b' "$2" > "$W/t.asm"
    if "$ASM" "$W/t.asm" "$W/t.bin" >"$W/log" 2>&1; then got=ok; else got=err; fi
    bytes=; [ $got = ok ] && bytes=$(od -An -tx1 -v "$W/t.bin" | tr -d ' \n')
    if [ "$got" != "$1" ]; then echo "FAIL: .org case '$2' should be $1, was $got"; bad=$((bad+1));
    elif [ -n "$3" ] && [ "$bytes" != "$3" ]; then echo "FAIL: .org case '$2' assembled to $bytes, expected $3"; bad=$((bad+1)); fi
}
org ok  '.org $8000\nNOP\nNOP\n.org $8004\n.word $8000\n'  '000000000080'
org ok  '.org $8000\nNOP\n.org $8003\nBRK #1\n'               '0000006201'
org ok  '.org $8000\n.org $9000\nNOP\n'                         '00'
org ok  '.org $8000\nNOP\n.org $8001\nNOP\n'                   '0000'
org err '.org $8000\nNOP\nNOP\nNOP\n.org $8001\nNOP\n'
rm -rf "$W"
[ $bad -eq 0 ] && echo "all assembler operand checks passed"
[ $bad -eq 0 ]
