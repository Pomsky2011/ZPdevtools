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
| `zp_serial.h` | the debug serial port: `zp_serial_putc/puts/getc/ready/flush`, `zp_is_dev_unit()` (a UART on `zeropoint_sdl --dev-unit`, a no-op on a retail unit) |
| `zp_pad.h`   | the controllers: `zp_pad_dir/ctrl/btn(player)`, `zp_pad_dx/dy(player)` (-1, 0, 1), the `ZP_DIR_*` `ZP_CTRL_*` `ZP_BTN_*` bits |
| `zp_layers.h` | the **layers microcode library**: two scrolling tile layers, sprites with three priorities, a window layer (see below) |
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
| `crt0_entry.asm` | Entrypoint-mode startup: sets 16-bit mode and a stack (`S = $F000`), enters `main` with `BRA`. `.include` at the top of your program. |
| `crt0_rom.asm`   | ROM-mode startup: fuller bring-up (CLD, defined flags) then `main`. |
| `zp_mmio.asm`    | `zp_peek8` / `zp_poke8`, implemented with absolute-long-indexed access to the RAM/APU/PPU/IO banks. |
| `zp_cop.asm`     | `zp_cop3` / `zp_cop32` (any `COP` signature, built at run time) plus the typed Boot ROM service wrappers. Needs the real Boot ROM (`--boot`) for `$F0-$FF`. |
| `zp_dma.asm`     | `zp_dma_load` for hand-written asm (boot ROM, no C runtime). |
| `zp_long.asm`    | 32-bit runtime for the C compilers (`long` add/sub/mul/div/mod, shifts, bitwise, compares). Link it into any program that does `long` arithmetic. Tested by `ZeroPoint/bin/test_zp_long`. |

**Two things that only go wrong on the real Boot ROM** (the native boot hides both, so test with `--boot` before calling anything done; `examples/layers-game`
is checked both ways by `tests/layers/run.sh`):

- **The machine state at entry.** The Boot ROM hands off with `DB = $80`, `S = $01FF` and `Y = $FFFE` (`ZeroPoint/docs/boot-handoff.md`); the native boot leaves
  `DB = $00`. A `JMP addr` takes its bank from `DB`, not `PB`, so a startup that ends in `JMP main` jumps into RAM on the real Boot ROM and runs garbage: `crt0_*.asm`
  use `BRA main` (the current bank, whatever `DB` is) and neither compiler emits `JMP`. A 512-byte stack is also too small for a C call chain a few functions
  deep, so the crt0s set `S`. If you write your own startup, do both.
- **Signing a ROM bigger than a few KB.** `zpbuild` without `--codesize` signs the original way: the Boot ROM computes one SHA-256 over the whole payload *in software*,
  which for a megabyte never finishes (the lightning splash just stays on screen, and it looks like an unsigned ROM). Use `--codesize N --trailer 2`: the console's BLAKE2s
  engine then checks the whole ROM in a fraction of a second. `N` must cover your CPU program (it starts at `$8000`); the layers example computes it from the size of `game.bin`.
  The native boot does not hash at all, so it runs either. See [linker.md](linker.md), "Trailer versions".

The C calling convention, shared by both compilers, is in [abi.md](abi.md): 16-bit mode, the first three words of arguments in
`A`, `X` and `Y`, the rest pushed right to left, the result in `A` (or `A:X` for 32 bits). `int` is 16 bits, `long` 32.

**Compilers.** Both `zpcc` and `def88186cc` are regression-tested by `tests/cc/run.sh`, which compiles each test with the host
gcc and with both compilers and compares what they record when run in the emulator. Arrays, pointers and structs work in
both: a pointer is a 16-bit offset into bank `$80`, and locals live in the stack frame. `zpcc` global variables are ROM
(readable by name, not writable or addressable); `def88186cc` has no globals. Neither accepts string literals as pointers
or struct values. The exact list for `def88186cc` is in `c_compiler/README.md`.

## Layers library — `zp_layers.h`

A game that wants scrolling tile layers and sprites does not have to speak the layers microcode's protocol (`ZPdevtools/docs/ppu/layers.md`):
`zp_layers.h` boots the microcode, loads palette, tiles, maps and cells, and once per frame builds the sprite stream and flips it in.
Header only, both compilers; `examples/layers-game/` is a complete small game (a pad-driven hero, two parallax layers, a bird flying
*between* the layers, a text box in the window layer) and `tests/layers/run.sh` checks its every pixel on both compilers.

```c
#include <zeropoint/zeropoint.h>
#include <zeropoint/zp_layers.h>

int main()
{
    zpl_init(0x10, 0x0000, 13);                 /* the microcode: 13 pages of ROM at $10:0000 (zplink --ppu layers.bin@0x100000) */
    zpl_palette_load(0x110000UL);               /* 512 bytes */
    zpl_tiles_load(0, 0, 0x110200UL, 5);        /* bank 0, ids 0..4, 64 bytes each */
    zpl_map_load(0, 0x111000UL);                /* BG0: 64 x 64 cells, 16 KiB */
    zpl_cells_load(0, 0x110C00UL, 4);           /* the four cells of a 2x2 sprite */
    zpl_start();                                /* PPU runs, 60 Hz timer on */
    zpl_sprite_set(0, 100, 60, 2, 2, 0, 0);     /* slot, x, y, w, h (tiles), first cell, flags */
    for (;;) {
        zpl_scroll(0, 1, 0);
        zpl_frame();                            /* wait for the 60 Hz tick, send the frame */
    }
}
```

What it does and the rules it asks for:

- **Everything the PPU draws from goes in before `zpl_start()`** (palette, tiles, maps, cells): the PPU serves tile uploads only until it is told to run.
- **Sprites** have slots (`ZPL_MAX_SPRITES`, 64), later slots draw over earlier ones, `flags` = `ZPL_SPR_HFLIP | VFLIP` and one of
  `ZPL_SPR_PRIO_MID` (between the layers) / `ZPL_SPR_PRIO_BACK` (behind BG0, which must then be transparent: `zpl_layer_transparent(1, 1)`).
  At most 15 pieces fit a band and 278 a frame; what does not fit is dropped and counted (`zpl_dropped()`).
- **The window layer** (`zpl_window_box`, `zpl_window_cells_load`, `zpl_window_glass_fill`): textboxes and HUD, never scrolling.
- **State is a block of Work RAM** at `ZPL_RAM` (default `$80:4000`, 6.4 KB; define it before the include to move it), because neither C compiler has
  writable globals. Keep your own data out of it.
- **It costs CPU.** The compilers generate unoptimised code, and building a frame costs about **12k CPU cycles per sprite on `zpcc` and 10k on
  `def88186cc`** (a 60 Hz frame is ~283k), so the builder alone can handle about 20 sprites a frame and a game needs room for its own logic. The stream is
  only rebuilt when a sprite changed (`zpl_sprite_move` and friends mark it), so a scrolling level with a few fixed sprites is nearly free. Hand-written
  assembly for the two hot loops (`zpl_pass`, `zpl_layout`) is the obvious next step if a game needs more.
- **Tests:** `tests/cc/t16_layers_stream.c` and `t17_layers_window.c` run the builders on both compilers and compare byte for byte with the host reference
  (`ucode/layers/zp_layers_sprites.h`, `zp_layers_window.h`); `t18_layers_dirty.c` the rebuild shortcut; `tests/layers/run.sh` the whole thing on the emulator.

**Two things the compilers and runtime make you work around** (they shaped the library, and are in `TODO.md` as bugs): `zp_peek8` cannot read ROM above
bank 0 (it reads bank 0 at the same offset), so ROM data reaches the PPU by DMA through a staging buffer, never by `zp_peek8(rom_addr)`; and `def88186cc` crashes
on a call result cast straight to a struct pointer and mis-scales `(struct S*)p + i`, so the library casts a variable first. Write your own code the same way if you
use struct pointers into fixed RAM with that compiler.

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
[`zplink`](linker.md) into an ELF for debugging and a signed ROM for shipping.
A complete worked example is in `examples/link/` (`make elf` links, `make rom` signs).
