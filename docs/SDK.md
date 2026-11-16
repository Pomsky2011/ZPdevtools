# ZeroPoint C SDK

A set of C headers plus a small runtime (`libzp`) that make ZeroPoint homebrew
in C an order of magnitude easier. Everything compiles with **both** ZeroPoint C
compilers:

- **zpcc** (clang/LLVM backend) — the modern path.
- **def88186cc** (MS-DOS 4.01+ / Turbo C-class) — the retro path, via the
  `ZP_CC_DOS` dialect mode.

## Headers — `include/zeropoint/`

Include the umbrella and you get everything:

```c
#include <zeropoint/zeropoint.h>
```

| Header | Contents |
|--------|----------|
| `zp_types.h` | Fixed-width types (`zpu8/16/32`, `zpaddr`), compiler adaptation macros |
| `zp_mmio.h`  | Full 24-bit memory map, every MMIO register address, `zp_peek8/poke8/peek16/poke16` |
| `zp_dma.h`   | DMA modes, `zp_dma_kick`, chunked `zp_dma_load` / `zp_dma_fill` |
| `zp_ppu.h`   | PPU window, VOC registers, `zp_ppu_boot` / `zp_ppu_upload` / start/reset |
| `zp_apu.h`   | APU window, `zp_apu_boot` / `zp_apu_upload_samples`, rombank, control |
| `zp_timer.h` | 8 hardware timers (`ZP_TMR_*`), enable/IRQ/status/wait helpers |
| `zp_cop.h`   | Every `COP` signature: generic `zp_cop3` / `zp_cop32`, Boot ROM service wrappers (`zp_sin`, `zp_sqrt`, `zp_hypot`, `zp_boot_ppu_load`, `zp_verify_chunk`, ...) |

Everything funnels down to two hardware primitives, `zp_peek8` / `zp_poke8`,
supplied by the runtime (`lib/zp_mmio.asm`); the `COP` calls come from `lib/zp_cop.asm`.

### Example

```c
#include <zeropoint/zeropoint.h>

int main()                        /* main(), not main(void), for def88186cc */
{
    zp_ppu_boot(0x100000UL, 2048, 0);   /* DMA PPU microcode from ROM, start it   */
    zp_apu_boot(0x200000UL, 1024, 1);   /* DMA APU program from ROM, run it        */
    zp_timer_enable(ZP_TMR_VBLANK);     /* 60 Hz frame timer                       */
    for (;;) {
        zp_timer_wait(ZP_TMR_VBLANK);   /* wait for V-blank                        */
        /* per-frame logic */
    }
}
```

**Warning: the APU is not reset when the cartridge starts.** It holds whatever the BIOS jingle engine left (different for
the real APU BIOS and the HLE one), so a program started by only writing the PC can corrupt itself. `zp_apu_boot()` resets
before it uploads, but leaves the APU running: the BIOS engine restarts and, with the real APU BIOS, has set its own SP/DP/flags
~0.6 ms later, so a long upload inherits that state again. For a long upload, or a hand-written loader, write `APUCTRL` =
`$03` (reset **and halt**), upload, set the PC, then `$04` (resume); see `ZeroPoint/docs/boot-handoff.md`.

Compile:

```sh
# modern (clang)
zpcc       -Iinclude               main.c -o main.asm
# retro (MS-DOS)
def88186cc -DZP_CC_DOS -Iinclude   main.c -o main.asm
# then
cpuasm main.asm main.bin
```

## DMA-based loaders

Assets (PPU microcode/tiles, APU programs/samples) live in CPU ROM and are
pulled into the PPU (`$B0`) and APU (`$A0`) windows by DMA. The controller moves
256–1024 bytes per transfer; the loaders split larger regions automatically and
round up to the 256-byte granularity (pad your blobs to 256):

```c
zp_ppu_upload(rom_gfx, ZP_PPU_CODE, gfx_len, /*chan*/0);   /* -> PPU window */
zp_apu_upload_samples(rom_smp, smp_len, /*chan*/1);        /* -> APU SST    */
zp_dma_fill(ZP_RAM_BASE, 0x00, 0x1000, /*chan*/2);         /* clear RAM     */
```

The loaders are written array-free (sequential register writes rather than a
`cfg[]` buffer) so they lower on def88186cc's backend too.

## Runtime — `lib/`

| File | Role |
|------|------|
| `crt0_entry.asm` | Entrypoint-mode startup: sets 16-bit mode, slides into `main`. `.include` at the top of your program. |
| `crt0_rom.asm`   | ROM-mode startup: fuller bring-up (CLD, defined flags) then `main`. |
| `zp_mmio.asm`    | `zp_peek8` / `zp_poke8`, implemented with absolute-long-indexed access to the RAM/APU/PPU/IO banks. |
| `zp_cop.asm`     | `zp_cop3` / `zp_cop32` (any `COP` signature, built at run time) plus the typed Boot ROM service wrappers. Needs the real Boot ROM (`--boot`) for `$F0-$FF`. |
| `zp_dma.asm`     | `zp_dma_load` for hand-written asm (boot ROM, no C runtime). |
| `zp_long.asm`    | 32-bit runtime for the C compilers (`long` add/sub/mul/div/mod, shifts, bitwise, compares). Link it into any program that does `long` arithmetic. Tested by `ZeroPoint/bin/test_zp_long`. |

The C ABI (shared by both compilers) is 16-bit mode: the first three register words (A, X, Y)
carry the leading arguments - a 32-bit argument takes two (A:X or X:Y, low word first) - and
the first argument that does not fit, and every later one, is pushed right to left. The result
is in A, or A:X (low, high) for a 32-bit value. `int` is 16 bits, `long` 32. `S` points at the
next free stack byte, so the top of the stack is `S+1`. See `docs/cpu/conventions.txt`.

**Compilers.** Both `zpcc` and `def88186cc` are regression-tested by `tests/cc/run.sh`, which
compiles each test with the host gcc and with both compilers and compares what they record when
run in the emulator. Known gaps (all three share one cause, the CPU has no register-indirect
addressing outside bank 0): array indexing, pointer dereference and struct members only reach
bank $00, and global variables are not supported by `def88186cc`.

## COP calls — `zp_cop.h`

A `COP` signature is an instruction operand, so it can't be a plain C argument;
`zp_cop.asm` writes `COP #sig / SDB #db / RTL` into scratch Work RAM and calls it.
That reaches every signature: the Boot ROM services `$F0-$FF` (math, PPU load,
chunk verify — `ZeroPoint/docs/boot-math-services.md`) and the cartridge's own
`$00-$EF`. It restores DB, which `COP #$FF` would otherwise clobber.

```c
zps16 s = zp_sin(0x4000);                 /* $7FFF */
zpu16 d = zp_hypot(3, 4);                 /* 5     */
zpu16 r = zp_cop3(0xF5, 0x4240, 0x000F);  /* any signature: A = sig, then the service's A and X */
```

Not re-entrant (fixed scratch at `$80:2B00`). `zp_cop32(sig, a, x, y)` takes all of A, X, Y
(the 4th argument is on the stack) and returns A:X; `zp_cop3(sig, a, x)` is the
three-register form. Tested by `ZeroPoint/bin/test_zp_cop` and `tests/cc/t06_cop.c`.

## def88186cc dialect (`ZP_CC_DOS`)

def88186cc accepts a narrow C subset. Define `ZP_CC_DOS` (via `-DZP_CC_DOS`) and
the headers adapt automatically:

- Types are `#define`d to builtin spellings (it can't use a `typedef` name as a
  type).
- `ZP_INLINE` becomes empty (no `static`), `ZP_VOID`/`ZP_CONST` drop `void`
  params / `const`.
- Prototypes are omitted (it resolves calls via implicit declaration; the
  definitions come from `libzp` at link time).

Write your own `main` as `int main()` and avoid `(void)`, `const`, and array
parameters in code that def88186cc must compile.

Building this SDK required several fixes to `def88186cc`'s preprocessor (nested
include search dirs, macro comment stripping, cross-include macro propagation,
recursive macro expansion, integer suffixes) and `-I`/`-D` forwarding in
`zpcc`. See the repo history and `c_compiler/preprocessor.c`.

## Linking

Once you have `main.bin` (+ PPU/APU blobs), link with
[`zplink`](LINKER.md) into an ELF for debugging and a signed ROM for shipping.
A complete worked example is in `examples/link/` (`make entry`, `make rom`).
