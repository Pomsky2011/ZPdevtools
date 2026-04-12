# Multi-File Assembly Guide

## DEF88186 CPU Assembler (cpuasm)

The `cpuasm` assembler now supports multi-file projects via the `.include` directive, allowing you to organize your assembly code into reusable modules.

### Usage

```asm
.include "filename.asm"
.include "path/to/file.asm"
```

### Features

- **Recursive Includes**: Included files can include other files
- **Quote Support**: Filenames can be quoted or unquoted
- **Relative Paths**: Paths are relative to the current working directory
- **Label Sharing**: All labels are globally accessible across files
- **Two-Pass Assembly**: Works seamlessly with forward references

### Example Project Structure

```
project/
├── main.asm           # Main program
├── math_lib.asm       # Math routines
├── hardware.asm       # Hardware I/O
└── build.sh           # Build script
```

### Example: math_lib.asm

```asm
; math_lib.asm - Reusable math routines

; Add function: adds A and value at $10
add_func:
    CLC
    ADC $10
    RTS

; Multiply by 2
mul2:
    ASL A
    RTS

; Constants
math_constant:
    .byte 42
```

### Example: hardware.asm

```asm
; hardware.asm - Hardware I/O routines

; Write byte to screen
write_screen:
    STA $2001
    RTS

; Read input
read_input:
    LDA $3000
    RTS
```

### Example: main.asm

```asm
; main.asm - Main program

.org $8000

; Include library files
.include "math_lib.asm"
.include "hardware.asm"

; Main program
main:
    REP #$30        ; 16-bit mode

    ; Use math library
    LDA #$0042
    JSR mul2

    ; Use hardware library
    JSR write_screen

    ; Loop
loop:
    JSR read_input
    CMP #$00
    BEQ loop

    JMP loop

; Reset vector
.org $FFFE
.word main
```

### Building

```bash
# Simple build
cpuasm main.asm program.bin

# The assembler will automatically include all referenced files
```

### Build Script Example

```bash
#!/bin/bash
# build.sh

echo "Assembling multi-file project..."
../../../cpuasm main.asm output.bin

if [ $? -eq 0 ]; then
    echo "Build successful! Binary: output.bin"
    ls -lh output.bin
else
    echo "Build failed!"
    exit 1
fi
```

### Best Practices

1. **Organize by Function**: Separate code into logical modules
   - `math.asm` - Math routines
   - `graphics.asm` - Display functions
   - `audio.asm` - Sound functions
   - `input.asm` - Input handling

2. **Use Comments**: Document what each file provides
   ```asm
   ; math.asm - Mathematical functions
   ; Exports: add_func, sub_func, mul2, div2
   ```

3. **Avoid Circular Includes**: Don't have files include each other
   ```
   ✗ main.asm includes lib.asm
   ✗ lib.asm includes main.asm  (BAD!)
   ```

4. **Include Once**: Each file should be included only once
   ```asm
   ; main.asm
   .include "lib.asm"    ; OK
   .include "lib.asm"    ; Duplicate - will assemble twice!
   ```

5. **Use Relative Paths**: Keep includes relative to project root
   ```asm
   .include "libs/math.asm"      ; Good
   .include "/abs/path/math.asm" ; Works but not portable
   ```

### Integration with C Compiler

The C compiler can generate assembly that uses includes:

```bash
# Compile C to assembly
def88186cc -I./include main.c lib.c -o combined.asm

# Assemble with cpuasm
cpuasm combined.asm program.bin
```

Or use the `-bin` flag for automatic assembly:

```bash
# One-step compilation
def88186cc main.c lib.c -bin -o program.bin
```

### Advanced: Library Organization

Create a common library structure:

```
ZPdevtools/
├── lib/
│   ├── stdlib.asm     # Standard library
│   ├── math.asm       # Math functions
│   ├── string.asm     # String functions
│   └── io.asm         # I/O functions
└── examples/
    └── project/
        └── main.asm   # Includes from ../../../lib/
```

```asm
; main.asm
.include "../../lib/stdlib.asm"
.include "../../lib/math.asm"

main:
    JSR stdlib_init
    ; ... your code ...
```

### Limitations

1. **No Include Guards**: Files can be included multiple times
   - Workaround: Only include each file once manually

2. **No Search Paths**: Must use relative or absolute paths
   - Workaround: Keep organized directory structure

3. **Global Namespace**: All labels are global
   - Use prefixes to avoid conflicts: `math_add`, `gfx_draw`, etc.

### Troubleshooting

**Problem**: "Cannot open input file"
```
Solution: Check file path is correct relative to current directory
```

**Problem**: Duplicate label errors
```
Solution: Each label must be unique across all files
Use prefixes: sprite_init, enemy_init (not just init)
```

**Problem**: Forward reference errors
```
Solution: The two-pass assembler handles forward refs automatically
Make sure labels are defined somewhere in included files
```

### Complete Example

See `examples/cpu/` for a working multi-file project:
- `math_lib.asm` - Math routines
- `hardware.asm` - I/O functions
- `main_multifile.asm` - Main program with includes

Build it:
```bash
cd examples/cpu
../../cpuasm main_multifile.asm test.bin
```

## Future Enhancements

Planned features for future releases:

- [ ] `.equ` directive for named constants
- [ ] Include guards (`#pragma once` equivalent)
- [ ] Search path support (`-I` flag)
- [ ] Conditional assembly (`.ifdef`, `.ifndef`)
- [ ] Macro definitions (`.macro` / `.endmacro`)

---

Built with [Claude Code](https://claude.com/claude-code)

Co-Authored-By: Claude <noreply@anthropic.com>
