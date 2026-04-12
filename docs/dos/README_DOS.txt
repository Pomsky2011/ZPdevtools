================================================================================
ZeroPoint Development Tools - MS-DOS 4.01+ Compatibility Guide
================================================================================

SYSTEM REQUIREMENTS
================================================================================

Minimum:
  - MS-DOS 4.01 or higher
  - Intel 80286 processor (or compatible)
  - 2 MB RAM
  - 100 MB hard disk space
  - Turbo C 2.0 or Microsoft C 6.0 compiler

Recommended:
  - MS-DOS 5.0 or higher
  - Intel 80386 processor
  - 4 MB RAM
  - 200 MB hard disk space
  - Turbo C 3.0 or Microsoft C 7.0 compiler

BUILDING ON MS-DOS
================================================================================

1. Extract the ZPdevtools directory to your DOS machine
   Example: C:\ZPTOOLS

2. Ensure your compiler is in the PATH:

   For Turbo C:
     SET PATH=C:\TC\BIN;%PATH%

   For Microsoft C:
     SET PATH=C:\MSC\BIN;%PATH%

3. Build all tools:

   For Turbo C:
     make -f Makefile.dos

   For Microsoft C:
     nmake /f Makefile.dos COMPILER=MSC

4. Tools will be built in the BIN subdirectory

MEMORY MODELS
================================================================================

The tools use different memory models based on their requirements:

SMALL MODEL (< 64KB code + < 64KB data):
  - Simple utilities that process small files
  - Fastest execution, smallest memory footprint

COMPACT MODEL (< 64KB code + > 64KB data):
  - Most assemblers and tools (DEFAULT)
  - Can process files larger than 64KB
  - Good balance of speed and capacity

LARGE MODEL (> 64KB code + > 64KB data):
  - C compiler (def88186cc)
  - Can compile complex programs
  - Uses more memory

MEMORY USAGE GUIDE (2 MB System)
================================================================================

MS-DOS 4.01 typically uses:
  - ~600 KB for DOS kernel and drivers
  - Leaves ~1.4 MB for applications

Tool memory requirements:
  - cpuasm, apuasm, ppuasm:     < 256 KB
  - rombuilder:                 < 384 KB
  - Disassemblers:              < 256 KB
  - Utilities (hexview):        < 128 KB
  - def88186cc (C compiler):    512 KB - 1 MB

Tips for low memory systems:
  - Close TSRs before building large programs
  - Use HIMEM.SYS to load DOS high (DOS 5.0+)
  - Don't assemble files larger than 32 KB on 2 MB systems
  - Split large projects into multiple files

FILE SIZE LIMITS
================================================================================

Due to MS-DOS and 16-bit limitations:

  - Maximum file size: 2 GB (FAT16 limit)
  - Practical limit for tools: 64 KB per file
  - Use .include directive to split large source files
  - ROM files can be up to 8 MB (loaded in chunks)

TOOL-SPECIFIC NOTES
================================================================================

ASSEMBLERS (cpuasm, apuasm, ppuasm):
  - Maximum 10,000 labels per file
  - Maximum 64 KB output per file
  - Maximum 1024 characters per line
  - Use .include for multi-file projects

  Example:
    cpuasm main.asm main.bin
    cpuasm graphics.asm gfx.bin
    rombuilder -cpu main.bin -ppu gfx.bin -o game.rom

C COMPILER (def88186cc):
  - Requires Flex/Bison (pre-built on modern system)
  - Maximum 8000 lines per source file
  - Maximum 2000 functions per program
  - Use large model (-ml flag with Turbo C)

  Build on DOS:
    Copy pre-generated lexer.c and parser.tab.c from modern build
    make -f Makefile.dos def88186cc.exe

ROM BUILDER (rombuilder):
  - Can build ROMs up to 8 MB
  - Reads input files in 32 KB chunks
  - Automatically pads to correct sizes

DISASSEMBLERS:
  - Can disassemble files up to 64 KB
  - Use hexview for files larger than 64 KB

KNOWN LIMITATIONS
================================================================================

1. No long filename support (8.3 format only)
   Use short, descriptive names: MAIN.ASM, not main_program.asm

2. Path length limited to 128 characters
   Keep directory structure shallow

3. No dynamic memory allocation checks
   Tools assume sufficient memory is available
   Use COMPACT or LARGE model if tools crash

4. Limited error recovery
   Syntax errors may cause tools to exit
   Save work frequently

5. No Unicode support
   ASCII only in source files

OPTIMIZATION TIPS
================================================================================

For faster builds on slow machines:

1. Use a RAM disk for temporary files
   Example (DOS 4.01):
     DEVICE=C:\DOS\RAMDRIVE.SYS 512 /E

   Then work in the RAM disk:
     COPY *.ASM D:\
     D:
     cpuasm main.asm main.bin

2. Disable unnecessary device drivers
   Remove unused drivers from CONFIG.SYS

3. Use disk caching
   SMARTDRV (DOS 5.0+) or equivalent

4. Defragment hard disk regularly
   Use DEFRAG or Norton Speed Disk

5. Split large projects into modules
   Assemble separately, link with ROM builder

TROUBLESHOOTING
================================================================================

"Out of memory" errors:
  - Close TSRs and device drivers
  - Use HIMEM.SYS to load DOS high
  - Split source files using .include
  - Reboot to clear memory fragmentation

"File not found" errors:
  - Check 8.3 filename format
  - Use backslashes in paths (C:\ZPTOOLS\MAIN.ASM)
  - Ensure files are in correct directory

"Stack overflow" errors:
  - Increase stack size in compiler options
  - Simplify deeply nested expressions
  - Split complex functions

Tool crashes or hangs:
  - Check source file for syntax errors
  - Ensure input files are not corrupted
  - Try smaller test file first
  - Recompile with debug symbols

CROSS-PLATFORM DEVELOPMENT
================================================================================

Recommended workflow:

1. Modern system (Linux/macOS/Windows):
   - Write and test code
   - Use modern editors and version control
   - Generate lexer/parser for C compiler

2. Transfer to DOS:
   - Use ZIP or TAR archives
   - Transfer via floppy, serial, or DOSBOX
   - Extract and build on DOS

3. Test on DOS:
   - Verify tools work correctly
   - Check memory usage
   - Test with large files

4. Transfer results back:
   - Copy binaries and ROM files to modern system
   - Test in ZeroPoint emulator

SUPPORT
================================================================================

For questions or issues with DOS builds:
  - Check ZPdevtools/docs/ directory
  - See examples in ZPdevtools/examples/
  - Consult your compiler documentation

MS-DOS 4.01 specific notes:
  - Uses FAT16 filesystem
  - 2 GB partition limit
  - No LFN (long filename) support
  - Limited extended memory support

================================================================================
