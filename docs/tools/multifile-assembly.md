# Multi-file assembly

All three assemblers support `.include`, which assembles another file at that point. All labels are global, so a project is
one flat namespace: prefix names (`gfx_draw`, `math_add`) to avoid clashes.

## cpuasm

```asm
.org $8000
.include "math_lib.asm"
.include "hardware.asm"

main:
    REP #$30
    LDA #$0042
    LDX #$0008
    JSR add_func
```

`examples/cpu/main_multifile.asm` is a working project (`math_lib.asm`, `hardware.asm`, `main_multifile.asm`). Build it from
its own directory:

```sh
cd examples/cpu
../../executables/cpuasm main_multifile.asm main.bin
```

What to know:

- **Includes are relative to the working directory**, not to the file that contains the `.include`. Assembling
  `examples/cpu/main_multifile.asm` from the repository root fails with `Cannot open input file` for each include, followed by
  an `Undefined label` for everything those files define. Run the assembler from the directory the paths are relative to.
- **Includes nest.** A file may include another, with the same working-directory rule for the path.
- **Labels are global and forward references work** (two passes).
- **A duplicate label is not an error.** Including a file twice assembles it twice with no complaint, so include each file once.
  There are no include guards.
- **`.org` is one origin shared by everything.** An `.org` inside an included file moves the position for the rest of the
  output, and padding goes in when it moves forward.
- **`.equ NAME value`** defines a constant (it must come before its first use).
- No macros, no conditional assembly, no search path (`-I`).

## apuasm

`.include "f.uas"` takes a path **relative to the including file**, so it works from any directory:

```text
.include "lib/a.uas"        ; found relative to this file
```

and an included file's own includes are relative to it. `.org` pads with zero words up to the address (never backward),
and `.rp` declares the page the next jumps are in ([../apu/programming-guide.md](../apu/programming-guide.md)).

## ppuasm

No `.include` and no other directives. A larger PPU program is generated, as the layers microcode is by
`ucode/layers/gen_layers.py`.
