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

Everything funnels down to two hardware primitives, `zp_peek8` / `zp_poke8`,
supplied by the runtime (`lib/zp_mmio.asm`).

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

The C ABI (shared by both compilers) is 16-bit mode, args in A/X/Y, return in A.

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
