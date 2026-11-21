# Building on MS-DOS

The tools are written in strict C89 so they can be built with Turbo C 2.01 or Microsoft C 6.0 on an 80286 with 2 MB of RAM.
**Only the C89 conformance is checked automatically.** The DOS build itself has not been run against a real Turbo C or
Microsoft C install, so treat `Makefile.dos` and `scripts/build-dos.bat` as untested.

## What is built

The sources keep their long names in `src/` (`cpudisasm.c`, `rominspect.c`, ...); there are no 8.3 renamed copies.
`src/compat.h` is the compatibility header the tools include in place of `<stdint.h>`: it detects Turbo C, Microsoft C or Watcom
and, on an old compiler, supplies the fixed-width types.

| Group | Tools |
|-------|-------|
| Assemblers | `cpuasm`, `ppuasm`, `apuasm` |
| Disassemblers | `cpudisasm`, `ppudisasm`, `apudisasm` |
| ROM tools | `rominspect`, `zplink`, `zpbuild` |
| Utilities | `hexview` |
| Compiler | `def88186cc` (`c_compiler/`), built from the pre-generated `lexer.c` and `parser.tab.c`, since Flex and Bison do not run on DOS |

`zpcc` needs clang/LLVM and is not built on DOS.

## Building

You need MS-DOS 4.01 or later, Turbo C 2.01 or Microsoft C 6.0, an 80286 or better, and 2 MB of RAM (more helps the
assemblers' buffers). From the ZPdevtools directory:

```bat
scripts\build-dos.bat
```

The script looks for Turbo C (`C:\TC`) or Microsoft C (`C:\MSC`) and runs `Makefile.dos`. To run it yourself:

```bat
make -f Makefile.dos                REM Turbo C
nmake /f Makefile.dos COMPILER=MSC  REM Microsoft C
```

The tools are built in the **compact** memory model (`-mc` for Turbo C, `/AC` for Microsoft C); the compiler uses the **large**
model. Executables land in `BIN\`. Each tool prints its usage when run with no arguments.

## The C89 rules

`tests/c89/check.sh` syntax-checks every tool and the compiler with

```text
gcc -std=c89 -pedantic -Wdeclaration-after-statement -Wno-long-long -fsyntax-only
```

and is part of `make check`. At the time of writing all of them pass ("all C89-clean"). The rules it enforces, which any new
code in `src/` and `c_compiler/` must follow:

- **Declarations come first in a block.** No declaration after a statement, and no declaration in a `for` header.
- **`/* */` comments only.** No `//`.
- **No variable-length arrays, no designated initializers, no `inline`.**
- **Integer types come from `src/compat.h`**, not `<stdint.h>`.
- **Print `unsigned long` with `%lu`**, not `%zu`; use `sprintf`, not `snprintf`.

DOS adds a few more things the compiler check cannot see: open binary files in binary mode (`"rb"`, `"wb"`), expect `\` as the
path separator and CR LF line endings, and keep file names to 8.3 characters when they are created on DOS.

## Checking on a modern host

```sh
sh tests/c89/check.sh      # just the conformance check
make check                 # build the tools, then the assembler, linker, disassembler and C89 checks
```
