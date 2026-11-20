# ZeroPoint DevTools - MS-DOS 4.01 Build Guide

The development tools are written in strict C89 (`tests/c89/check.sh` enforces it with
`gcc -std=c89 -pedantic -Wdeclaration-after-statement`) so they can be built with Turbo C 2.01 or Microsoft C 6.0 on an 80286
with 2 MB RAM. Only the C89 conformance is checked automatically; the DOS build itself is not
exercised in CI, so treat `Makefile.dos` as untested against a real Turbo C / MSC install.

## Source files

The sources keep their long names (`cpudisasm.c`, `rombuilder.c`, ...) in `src/`; there are no 8.3
renamed copies. `src/compat.h` is the C89/DOS compatibility header most tools include.

| Group | Tools |
|---|---|
| Assemblers | `cpuasm`, `ppuasm`, `apuasm` |
| Disassemblers | `cpudisasm`, `ppudisasm`, `apudisasm` |
| ROM tools | `rombuilder`, `rominspect`, `zplink`, `zpbuild` |
| Utilities | `hexview`, `wav2mmp` |
| Compiler | `def88186cc` (`c_compiler/`; needs the pre-generated `lexer.c` / `parser.tab.c`, since Flex and Bison are not available on DOS) |

## Building on DOS

Requirements: MS-DOS 4.01+, Turbo C 2.01 or Microsoft C 6.0, an 80286 or better, 2 MB RAM (16 MB
recommended for the assemblers' buffers).

From the ZPdevtools directory:

```batch
scripts\build-dos.bat
```

The script finds Turbo C (`C:\TC`) or Microsoft C (`C:\MSC`), then runs `Makefile.dos`. To run it yourself:

```batch
make -f Makefile.dos                REM Turbo C
nmake /f Makefile.dos COMPILER=MSC  REM Microsoft C
```

Everything is built with the compact memory model (`-mc`, `/AC`); the compiler uses the large model.
Executables land in `BIN\`. Each tool prints usage when run with no arguments.

## Cross-checking on a modern host

```bash
make check        # builds the tools, runs the assembler/linker/disassembler tests and the C89 check
```

The C89 check compiles every tool and the compiler with `-std=c89 -pedantic -Wdeclaration-after-statement`: no `//` comments, no
declarations after statements, `%lu` instead of `%zu`, `sprintf` instead of `snprintf`.

See `README_DOS.txt` and `C89_PORTING_GUIDE.txt` in this directory for the
porting rules and the background.
