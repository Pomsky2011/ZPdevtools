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
# an index register is an index register in any case, with or without a space (lower case used to drop it silently)
try ok  'tbl: LDA >tbl,x'      '3e000280'
try ok  'LDA $123456,x'        '3e563412'
try ok  'LDA $1234,x'          '393412'
try ok  'LDA $1234, X'         '393412'
try ok  'LDA 5,s'              '3f05'
# Forms cpudisasm prints and cpuasm has to read back: '!' / '>' override the by-value choice of direct page / long, block
# moves take two bank bytes in the order written (CPU::opMVN fetches the source bank first), DIV X,Y.
try ok  'MVP $7E,$7F'        '687e7f'
try ok  'MVN $01 , $02'      '670102'
try err 'MVN $01'
try err 'MVN $100,$02'
try err 'MVN $01,$02,$03'
try ok  'JMP ![$0005]'       '0e0500'
try ok  'JMP [$1234]'        '0e3412'
try err 'JMP [$0005]'
try ok  'LDA ![$10]'         '3510'
try ok  'LDA [$10]'          '3510'
try ok  'JMP >$001234'       '10341200'
try ok  'BEQ !$000005'       '0b050000'
try ok  'DIV X,Y'            '92'
# INC.L / DEC.L: the 6-cycle duplicates of INC/DEC absolute (opcodes $6E / $76), 16-bit operand like their twins
try ok  'INC $1234'          '6a3412'
try ok  'INC.L $1234'        '6e3412'
try ok  'DEC $1234'          '723412'
try ok  'DEC.L $1234'        '763412'
try ok  'inc.l $1234'        '6e3412'
try ok  'INC.L !$0005'       '6e0500'
try ok  'tbl: INC.L tbl'     '6e0002'
try err 'INC.L $12345'
try err 'INC.L #1'
try err 'INC.L'

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
# .byte / .word / .long take numbers, .EQU constants and labels (forward ones too); they used to drop a label without a word,
# and an unknown name or a value that does not fit is an error
try ok  'tbl: .word tbl'               '0002'
try ok  '.word fwd
fwd: NOP'                              '020200'
try ok  'NOP
tbl: .byte tbl'                        '0001'
try ok  'tbl: .long tbl'               '000280'
try ok  '.EQU K 7
.byte K, 1'                            '0701'
try ok  '.word 1, 2,3'                 '010002000300'
try ok  '.byte -1'                     'ff'
try err '.byte 256'
try err '.word 65536'
try err '.long $1000000'
try err '.byte nosuch'
try err '.word nosuch'
# the short spellings the opcode CSV lists
try ok  'LP #3'                        '130300'
try ok  'PUSHF'                        '2f'
try ok  'PPP'                          '33'
try ok  'INA'                          '69'
try ok  'DEA'                          '71'
try ok  'INA A'                        '69'
try ok  'dea a'                        '71'
try err 'INA X'
try ok  'JML [$1234]'                  '0e3412'
try ok  'JML ![$0005]'                 '0e0500'
try ok  'JML >$001234'                 '10341200'
# a label defined twice is an error (a file included twice used to assemble twice)
try err 'a: NOP
a: NOP'
# .include: relative to the including file first, then the working directory; a self-include is an error, not a crash
mkdir -p "$W/sub"
printf 'inner: NOP\n' > "$W/sub/inc.asm"
printf '.org $0200\n.include "inc.asm"\n.word inner\n' > "$W/sub/main.asm"
printf '.org $0200\n.include "sub/inc.asm"\n' > "$W/cwdrel.asm"
printf '.org $0200\n.include "loop.asm"\n' > "$W/loop.asm"
inc() {      # expect (ok|err) dir file [expected bytes in hex]
    if (cd "$2" && "$ASM" "$3" "$W/i.bin" >"$W/log" 2>&1); then got=ok; else got=err; fi
    bytes=; [ $got = ok ] && bytes=$(od -An -tx1 -v "$W/i.bin" | tr -d ' \n')
    if [ "$got" != "$1" ]; then echo "FAIL: .include case $3 (from $2) should be $1, was $got"; bad=$((bad+1));
    elif [ -n "$4" ] && [ "$bytes" != "$4" ]; then echo "FAIL: .include case $3 assembled to $bytes, expected $4"; bad=$((bad+1)); fi
}
inc ok  "$W"     sub/main.asm  '000002'
inc ok  "$W/sub" main.asm      '000002'
inc ok  "$W"     cwdrel.asm    '00'
inc err "$W"     loop.asm
cp "$W/sub/inc.asm" "$W/twice_inc.asm"
printf '.org $0200\n.include "twice_inc.asm"\n.include "twice_inc.asm"\n' > "$W/twice.asm"
inc err "$W"     twice.asm
# ppuasm: 0b binary literals (0b101 used to read as 0 with no error), a literal with trailing junk is an error, and a
# SETBYTE number that does not fit a byte is an error rather than its low byte
PPUASM=${PPUASM:-$HERE/../../executables/ppuasm}
ppu() {      # expect (ok|err) "source" [source that must assemble to the same bytes]
    printf '%s\n' "$2" > "$W/p.asm"
    if "$PPUASM" "$W/p.asm" -o "$W/p.bin" >"$W/log" 2>&1; then got=ok; else got=err; fi
    if [ "$got" != "$1" ]; then echo "FAIL: ppuasm '$2' should be $1, was $got"; bad=$((bad+1)); return; fi
    [ -z "$3" ] && return
    cp "$W/p.bin" "$W/p1.bin"; printf '%s\n' "$3" > "$W/p.asm"
    "$PPUASM" "$W/p.asm" -o "$W/p.bin" >"$W/log" 2>&1
    cmp -s "$W/p1.bin" "$W/p.bin" || { echo "FAIL: ppuasm '$2' differs from '$3'"; bad=$((bad+1)); }
}
ppu ok  'SETBYTE 2, 0b101'            'SETBYTE 2, 5'
ppu ok  'SETBYTE 2, 0B11111111'       'SETBYTE 2, 255'
ppu ok  'SETBYTE 2, 0x1F'             'SETBYTE 2, 31'
ppu ok  'SETBYTE 2, $1F'              'SETBYTE 2, 31'
ppu ok  'SETBYTE 2, 255'
ppu ok  'SETBYTE 2, -1'
ppu ok  'SETBYTE 2, (0b11+1)*2'       'SETBYTE 2, 8'
ppu err 'SETBYTE 2, 0b102'
ppu err 'SETBYTE 2, 0b'
ppu err 'SETBYTE 2, 08'
ppu err 'SETBYTE 2, 12abc'
ppu err 'SETBYTE 2, 256'
ppu err 'SETBYTE 2, 0x1234'
ppu err 'SETBYTE 2, -129'
rm -rf "$W"
[ $bad -eq 0 ] && echo "all assembler operand checks passed"
[ $bad -eq 0 ]
