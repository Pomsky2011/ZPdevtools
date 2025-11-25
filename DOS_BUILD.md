# ZeroPoint DevTools - MS-DOS 4.01 Build Guide

All ZeroPoint development tools are fully compatible with MS-DOS 4.01+ and can be compiled with Turbo C 2.01.

## DOS-Compatible Source Files

Due to the 8.3 filename limitation in DOS, use these filenames:

### Assemblers
- `cpuasm.c` - DEF88186 CPU assembler
- `ppuasm.c` - PPU microcode assembler
- `apuasm.c` - APU program assembler

### Disassemblers
- `cpudasm.c` - CPU disassembler (use instead of cpudisasm.c)
- `ppudasm.c` - PPU disassembler (use instead of ppudisasm.c)
- `apudasm.c` - APU disassembler (use instead of apudisasm.c)

### Utilities
- `rombuild.c` - ROM packager (use instead of rombuilder.c)
- `hexview.c` - Hex viewer
- `rominsp.c` - ROM inspector (use instead of rominspect.c)
- `wav2mmp.c` - WAV to MMP converter

### Headers
- `compat.h` - C89/DOS compatibility header (required for all tools)

## Building on DOS

### Requirements
- MS-DOS 4.01 or higher
- Turbo C 2.01 (or compatible)
- Intel 80286 or higher
- 2 MB RAM minimum
- 16 MB RAM recommended (for malloc)

### Quick Build

1. Copy all `.c` files and `compat.h` to a DOS-accessible directory
2. Run the provided BUILD.BAT script

The BUILD.BAT script will:
- Set up Turbo C paths
- Create TURBOC.CFG with proper compiler flags
- Compile all 10 tools with the compact memory model (-mc)
- Report success/failure for each tool

### Manual Compilation

If BUILD.BAT doesn't work:

```batch
SET TC=C:\TC
SET PATH=%TC%;%PATH%

REM Create config file
ECHO -I%TC%\INCLUDE > TURBOC.CFG
ECHO -L%TC%\LIB >> TURBOC.CFG
ECHO -mc >> TURBOC.CFG

REM Compile each tool
TCC cpuasm.c
TCC ppuasm.c
TCC apuasm.c
TCC cpudasm.c
TCC ppudasm.c
TCC apudasm.c
TCC rombuild.c
TCC hexview.c
TCC rominsp.c
TCC wav2mmp.c
```

## Technical Notes

### Memory Model
All tools use the **compact memory model** (-mc):
- 64KB code segment
- Far data pointers (unlimited data)
- Optimal for DOS with large data structures

### Dynamic Allocation
Several tools use malloc() for large buffers:
- **ppuasm**: ~1MB for instruction/label tables
- **apuasm**: ~32KB for instruction buffer
- These allocations work in DOS with 16MB+ configured memory

### C89 Compliance
All code is strictly C89/ANSI C compliant:
- No C99 features (no `for (int i=0; ...)`, no `//` comments, etc.)
- Uses `sprintf` instead of `snprintf`
- Uses `%lu`/`%ld` instead of `%zu`/`%zd`
- All variable declarations at function start
- `char* argv[]` instead of `char** argv`

### Limitations
Due to 16-bit DOS constraints:
- Maximum file sizes: ~64KB for most operations
- Reduced array sizes compared to modern versions:
  - cpuasm: 256 labels, 16KB code buffer
  - ppuasm: 512 labels, 4096 instructions
  - apuasm: 256 labels, 16384 instructions

These limits are sufficient for typical ZeroPoint development.

## Testing

After compilation, test each tool:

```batch
apuasm
cpuasm
ppuasm
cpudasm
ppudasm
apudasm
rombuild
hexview
rominsp
wav2mmp
```

Each should display usage information.

## Troubleshooting

### "Out of memory" errors
Increase DOSBox memory:
```ini
[dosbox]
memsize=32
```

### "Too much global data"
This has been fixed by using malloc(). If you still see this, the Turbo C installation may be incorrect.

### "Cannot find include file"
Ensure TURBOC.CFG exists with proper paths, or add flags manually:
```batch
TCC -I%TC%\INCLUDE -L%TC%\LIB -mc filename.c
```

## Modern Build

On modern systems, compile with:
```bash
make
```

The Makefile automatically uses C89 mode and the compatibility header.

---

Built with [Claude Code](https://claude.com/claude-code)

Co-Authored-By: Claude <noreply@anthropic.com>
