# ZPdevtools TODO

## Critical Bugs

**NONE** — the loop-counter bug below was fixed in `32cd754` ("Fix critical
PPU loop bug - Correct Preset E instruction encoding", 2025-10-23).

### ✅ RESOLVED: Loop Counters Get Stuck (2025-10-19, fixed 2025-10-23)
**Root cause**: TARREG/SETBYTE/BUILD (Preset E) had wrong bit shifts in the
assembler — target-register field landed in the wrong bits of the suboperand,
so labels/jumps resolved to garbage addresses.
**Fix**: TARREG target_reg `<<6`→`<<8`, byte_sel `<<5`→`<<7`; SETBYTE target_reg
`<<6`→`<<8`; BUILD t1 `<<6`→`<<8`, t2 `<<4`→`<<6`.

## Recent Changes

### Complete C89/ANSI C Toolchain - Major Update (2025-10-23)
**Status**: ✅ COMPLETE - Full C89 compiler with preprocessor, inline assembly, and ROM building

#### New Tools
- ✅ **C Preprocessor** (preprocessor.c/h) - Full C89 preprocessor implementation
  - `#include` directive with nested includes and search paths
  - `#define` macros (simple and function-like with parameter substitution)
  - Conditional compilation: `#ifdef`, `#ifndef`, `#if`, `#elif`, `#else`, `#endif`
  - `#undef` for macro removal
  - `#pragma` support (currently ignored)

- ✅ **Inline Assembly Support** - Native assembly in C code
  - `__asm__("instruction")` and `asm("instruction")` syntax
  - AST integration with new AST_INLINE_ASM node type
  - Direct emission to assembly output

- ✅ **Multi-file Compilation** - GCC-style command-line interface
  - `-I <path>` for include directories
  - `-D MACRO[=value]` for command-line macros
  - `-E` for preprocessor-only output
  - `-bin` for complete compile + link + assemble workflow
  - Header file (.h) support

- ✅ **ROM Builder** (rombuilder.c) - Combine binaries into ROM files
  - Merges CPU, PPU, APU binaries into single ROM
  - Configurable base addresses (default: CPU@0x8000, PPU@0x100000, APU@0x200000)
  - Custom entry points
  - Metadata generation (INI-style .txt file)
  - Verbose output option

- ✅ **Assembly .include Directive** - Multi-file assembly projects
  - `.include "filename.asm"` support in cpuasm
  - Recursive include processing
  - Works in both assembly passes

- ✅ **ppuasm Rename** - Consistent naming
  - Renamed zpasm → ppuasm for clarity
  - Updated all documentation references

#### Documentation
- ✅ **ASM_MULTIFILE_GUIDE.md** - Assembly multi-file documentation
- ✅ **ROMBUILDER_GUIDE.md** - Complete ROM builder guide
- ✅ **Complete ROM example** - Full project structure with build script

#### Impact
Now a **complete professional toolchain**:
- C89/ANSI C compatibility with full preprocessor
- Inline assembly for hardware-specific code
- Multi-file projects with header files
- Complete build pipeline: C → ASM → BIN → ROM
- Professional command-line interface
- Ready for large-scale game development

### DEF88186 C Compiler - Major Feature Update (2025-10-22)
**Status**: ✅ COMPLETE - Full-featured C compiler ready for production

#### Phase 1 (Earlier Today)
- ✅ **Added 7 new CPU transfer instructions** (TSC, TCS, TAX, TXA, TAY, TCD, TDC)
- ✅ **Compound assignment operators**: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`
- ✅ **Increment/decrement operators**: `++`, `--` (prefix and postfix)
- ✅ **Break and continue statements** for loop control
- ✅ **Struct member assignment**: `p.x = value`, `ptr->member = value`
- ✅ **Updated cpuasm** to support all C compiler generated instructions
- ✅ **Added instruction support**: BRA, STA stack-relative, DIV/MUL X, .data/.code directives
- ✅ **Full toolchain working**: C → Assembly → Binary compilation complete

#### Phase 2 (Latest Update)
- ✅ **sizeof operator**: `sizeof(int)`, `sizeof(expr)` - compile-time size evaluation
- ✅ **String literals**: `"Hello, World!"` - basic string support with char* pointers
- ✅ **Character literals**: `'A'`, `'\n'`, `'\t'`, `'\0'` - full escape sequence support
- ✅ **Ternary operator**: `(a > b) ? a : b` - conditional expressions
- ✅ **Switch/case statements**: Full switch/case/default with break support
- ✅ **Multi-dimensional arrays**: `int matrix[3][4]`, `int cube[2][3][4]` - N-dimensional arrays
- ✅ **Better error messages**: Line and column numbers in all parse errors
- ✅ **5 new example programs**: sizeof, ternary, switch, multidim arrays, strings
- ✅ **Updated documentation**: Complete README.md with all new features

#### Impact
The C compiler now supports a **comprehensive subset of C** suitable for game development:
- All basic data types (int, char, void, struct, pointers, arrays)
- All operators (arithmetic, logical, bitwise, comparison, ternary, sizeof)
- Full control flow (if/else, while, for, switch/case, break, continue)
- Functions with recursion and proper calling conventions
- Hardware optimization (MUL/DIV, LOOP/LPEND)
- **16 working example programs** demonstrating all features

### R59/R60 Interrupt Documentation (2025-10-19)
- Updated README.md with R59/R60 interrupt register documentation
- Noted automatic return address push behavior
- Added warning about target register 0 and 1 corruption by shorthands

### Shorthand System (Previous)
- Added label-based jump shorthands (JMR, JMZ, JNZ, JNG, JMG)
- Added stack operation shorthands (PUSH, POP, JSR, RET)
- Added HLT shorthand (6-instruction infinite loop)
- Added variable aliases (e.g., `R10 = COUNTER`)

## Known Issues

1. **HLT is not HALT**: HLT expands to 6-instruction loop, not a real CPU halt
2. **Target Register Corruption**: Shorthands overwrite target registers 0 and 1
3. **No Immediate `MOV`**: Cannot do `MOV R0, #42` - PPU's real `MOV` opcode is
   hardware-defined as DP-addressed memory move only (no immediate encoding
   exists in the ISA); `SETBYTE` is the intended way to place an immediate
   into a register, and now supports full expressions (see CONST below) -
   not implemented as "MOV #imm" since that would require inventing an opcode
   the hardware doesn't have
4. **No Macro System**: No way to define reusable code blocks (Note: C compiler now has preprocessor macros)
5. ~~**No Include Files**: Cannot split code across multiple files~~ - ✅ FIXED: Added .include directive for assembly, #include for C

## Testing Needed

### General
- [ ] Test all shorthands with edge cases
- [ ] Test variable aliases with edge cases
- [ ] Verify interrupt handler examples work
- [ ] Test PUSH/POP/JSR/RET stack operations

## Recent Additions (2025-11-24)

### ✅ MS-DOS 4.01 Compatibility - COMPLETE
**Status**: COMPLETE - All tools compile and run on MS-DOS 4.01 with Turbo C 2.01

#### Achievement
Successfully ported all 10 ZeroPoint development tools to run on MS-DOS 4.01 (1988) with Turbo C 2.01 (1987) targeting Intel 80286 (1982) processors!

#### Tools Ported
- ✅ **cpuasm** - DEF88186 CPU assembler
- ✅ **ppuasm** - PPU microcode assembler
- ✅ **apuasm** - APU program assembler
- ✅ **cpudasm** - CPU disassembler (DOS name for cpudisasm)
- ✅ **ppudasm** - PPU disassembler (DOS name for ppudisasm)
- ✅ **apudasm** - APU disassembler (DOS name for apudisasm)
- ✅ **rombuild** - ROM builder (DOS name for rombuilder)
- ✅ **hexview** - Hex viewer
- ✅ **rominsp** - ROM inspector (DOS name for rominspect)
- ✅ **wav2mmp** - WAV to MMP converter

#### Technical Achievements
- ✅ **Strict C89/ANSI C compliance** - No C99 features
  - Variable declarations at function start
  - C-style comments only (/* */)
  - sprintf instead of snprintf
  - %lu/%ld instead of %zu/%zd
  - char* argv[] instead of char** argv
  - No variadic macros

- ✅ **Dynamic memory allocation** - Bypassed 64KB static data limit
  - ppuasm: ~1MB malloc for tables
  - apuasm: ~32KB malloc for instruction buffer
  - Uses compact memory model (-mc)

- ✅ **8.3 filename compatibility** - DOS filename limitations
  - Created symlinks for modern builds
  - Both naming schemes work seamlessly

- ✅ **DOS line endings** - CRLF format for all files
- ✅ **compat.h** - C89 compatibility header
- ✅ **BUILD.BAT** - Automated DOS build script
- ✅ **TURBOC.CFG** - Proper compiler configuration

#### Documentation
- ✅ **DOS_BUILD.md** - Complete DOS build guide
- ✅ **DOS_COMPATIBILITY.txt** - Technical summary
- ✅ **README.md** - Updated with DOS build section
- ✅ **Makefile** - Still works for modern builds

#### Impact
**Complete retro-compatible toolchain** - Same source code compiles on:
- Modern systems (macOS, Linux, Windows with GCC/Clang)
- MS-DOS 4.01+ with Turbo C 2.01
- Proven C89 portability to virtually any platform

## Recent Additions (2025-10-24)

### ✅ Development Tools Suite - COMPLETE
**Status**: COMPLETE - Professional debugging and analysis tools

#### New Tools Added
- ✅ **CPU Disassembler** (cpudisasm) - Disassemble DEF88186 binaries
  - Full 256 opcode support
  - Byte display (-b flag)
  - Comments mode (-c flag)
  - Range selection (-s/-e flags)

- ✅ **PPU Disassembler** (ppudisasm) - Disassemble PPU microcode
  - 15 basic + Preset E/F support
  - 16-bit big-endian decoding
  - Register operand display

- ✅ **APU Disassembler** (apudisasm) - Disassemble APU programs
  - All 47 APU instructions
  - 16-bit little-endian decoding
  - MMP/SST instruction support

- ✅ **ROM Inspector** (rominspect) - Analyze ZPB ROM files
  - Header information display
  - Component size verification
  - Checksum validation (-c flag)
  - Component extraction (-x flag)
  - Memory layout visualization (-v flag)

- ✅ **Hex Viewer** (hexview) - Smart binary file viewer
  - Customizable width and grouping
  - ASCII display
  - Range selection
  - File statistics (--analyze flag)

- ✅ **Enhanced Makefile** - Complete build system
  - Convenience targets (assemblers, disassemblers, rom-tools, utilities)
  - Help command (`make help`)
  - Clean and install targets

#### Documentation
- ✅ **DEV-TOOLS.md** - Complete development tools guide
  - Usage examples for all tools
  - Workflow guides
  - Tips and tricks
  - Troubleshooting section

#### Impact
Now a **complete professional development environment**:
- Full assembly/disassembly cycle
- ROM building and inspection
- Binary analysis and debugging
- 11 total development tools
- Comprehensive documentation

---

## Future Enhancements

### High Priority
- [x] ~~Fix loop bug~~ (CRITICAL) - ✅ DONE: fixed in 32cd754
- [ ] Add verbose/debug mode to assemblers
- [ ] Better error messages with line numbers

### Medium Priority
- [x] ~~Macro system~~ - ✅ DONE: C preprocessor has full macro support
- [x] ~~Include file support~~ - ✅ DONE: .include for assembly, #include for C
- [x] ~~Immediate value syntax (`MOV R0, #42`)~~ - not applicable: `MOV`'s
  hardware encoding has no immediate form; `SETBYTE` already covers this and
  now takes full expressions
- [x] ~~Constant definitions (`CONST SCREEN_WIDTH 256`)~~ - ✅ DONE in ppuasm
- [x] ~~Expression evaluation (`SETBYTE 2, SCREEN_WIDTH/2`)~~ - ✅ DONE in
  ppuasm: `+ - * /`, parentheses, unary minus, CONST names, and labels

### Low Priority
- [ ] Optimization passes
- [ ] Dead code elimination
- [ ] Peephole optimization
- [ ] Symbol table export for debugging

## Documentation Status

### Complete - PPU
- ✅ README.md with basic usage
- ✅ Instruction reference
- ✅ Shorthand documentation (docs/ppu/preset-e-and-shorthands.txt)
- ✅ Interrupt system (R59/R60)
- ✅ Target register warnings
- ✅ ppuasm renamed from zpasm (all docs updated)

### Complete - APU (2025-10-19)
- ✅ Complete APU/DSP documentation suite (8 files, ~100 KB)
- ✅ docs/apu/README.txt - Documentation index
- ✅ docs/apu/overview.txt - Architecture overview
- ✅ docs/apu/instruction-set.txt - All 41 instructions documented
- ✅ docs/apu/memory-map.txt - Complete memory layout
- ✅ docs/apu/registers.txt - Register reference
- ✅ docs/apu/sst.txt - Sample Storage System
- ✅ docs/apu/mmp.txt - Music Mixing Processor
- ✅ docs/apu/programming-guide.txt - 8 complete examples
- ✅ apuasm assembler fully implemented

### Complete - C Compiler & Toolchain (2025-10-23)
- ✅ README.md with complete C89 feature documentation
- ✅ ASM_MULTIFILE_GUIDE.md - Assembly multi-file guide
- ✅ ROMBUILDER_GUIDE.md - ROM builder documentation
- ✅ Complete ROM example project with build scripts
- ✅ Working examples for preprocessor and inline assembly

### Needed
- [ ] Troubleshooting guide
- [ ] Loop debugging guide
- [ ] Best practices document
- [ ] Performance optimization guide
- [ ] Example programs

## Examples Status

> Note: the loop-based examples below were blocked by the Preset E encoding
> bug, fixed in 32cd754 (2025-10-23). They should be re-verified against the
> current assembler rather than assumed broken.

### Working
- ✅ Simple programs without loops
- ✅ HLT detection

### Needs Re-verification (were blocked by the now-fixed loop bug)
- ⚠️ color_bars.asm
- ⚠️ color_bars_clean.asm
- ⚠️ gradient_demo_simple.asm
- ⚠️ gradient_test.asm
- ⚠️ All loop-based demos

### Not Tested
- ⚠️ Interrupt handler examples
- ⚠️ JSR/RET subroutines
- ⚠️ Complex nested loops
