#!/bin/sh
# The DOS-portable tools must stay strict C89 (Turbo C 2.0 / MS C): syntax-check them with gcc's strictest C89 mode.
# Usage: tests/c89/check.sh      (from anywhere; exit 0 = clean)
cd "$(dirname "$0")/../.." || exit 2
FLAGS="-std=c89 -pedantic -Wdeclaration-after-statement -Wno-long-long -fsyntax-only"
bad=0
( cd c_compiler && make parser.tab.c lexer.c >/dev/null 2>&1 )   # generated sources, checked too
for f in c_compiler/ast.c c_compiler/codegen.c c_compiler/main.c c_compiler/preprocessor.c \
         c_compiler/parser.tab.c c_compiler/lexer.c \
         src/apuasm.c src/apudisasm.c src/cpuasm.c src/cpudisasm.c src/hexview.c src/ppuasm.c \
         src/ppudisasm.c src/rombuilder.c src/rominspect.c src/wav2mmp.c src/zpbuild.c src/zplink.c; do
    [ -f "$f" ] || continue
    out=$(gcc $FLAGS -Ic_compiler -Isrc "$f" 2>&1 | grep -E 'error|warning')
    if [ -n "$out" ]; then echo "FAIL $f"; echo "$out" | head -5; bad=$((bad+1)); else echo "ok   $f"; fi
done
[ $bad -eq 0 ] && echo "all C89-clean" || echo "$bad file(s) not C89-clean"
[ $bad -eq 0 ]
