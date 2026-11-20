# Architecture

## Registers

| Register | Width | Reset | Notes |
|----------|-------|-------|-------|
| `A` | 16 | `$0000` | Accumulator. With `M=1` only the low byte is used and the high byte is left alone. |
| `X`, `Y` | 16 | `$0000` | Index registers. With `X=1` only the low byte is used for indexing, loads and arithmetic. |
| `SP` | 16 | `$01FF` | Stack pointer. Points at the **next free byte**; pushes store then decrement. |
| `D` | 16 | `$0000` | Direct-page base. Added to a one-byte operand to form an address in bank `$00`. |
| `PC` | 16 | `$0000` | Program counter. |
| `PB` | 8 | `$00` (`$E0` with a Boot ROM) | Program bank. Code runs from `PB:PC`. |
| `DB` | 8 | `$00` | Data bank. Supplies the bank byte for most absolute addresses. |
| `P` | 8 | `$34` | Status flags, below. |

There are also two hidden registers for the hardware loop (`LOOP`/`LPEND`): a counter and a loop start address. There is
only one pair, so loops cannot be nested.

Reset has **no vector**. The CPU starts at `PB:PC = $00:0000`, or at `$E0:0000` when a Boot ROM is mapped. The Boot ROM
verifies the cartridge and jumps to its entry point (read from `$D80049-$D8004B`); see
[memory-map.md](memory-map.md). Don't rely on the flags at your entry point: `lib/crt0_rom.asm` begins with `SEI`, `REP #$30`, `CLD`, `CLC` for that reason.

## Status flags

```text
bit:   7  6  5  4  3  2  1  0
flag:  N  V  M  X  D  I  Z  C
```

| Flag | Meaning |
|------|---------|
| `N` | Negative: top bit of the last result (8 or 16 bits wide, per M/X). |
| `V` | Overflow, set by `ADC`, `SBC`, `BIT` (bit 6 of the operand, or bit 14 in 16-bit mode) and `MUL`. |
| `M` | `1` = 8-bit accumulator and memory operations, `0` = 16-bit. Set at reset. |
| `X` | `1` = 8-bit index registers, `0` = 16-bit. Set at reset. |
| `D` | Decimal mode: `ADC` and `SBC` work in BCD. Nothing else is affected. |
| `I` | `1` blocks the maskable IRQ. Set at reset. |
| `Z` | Zero. |
| `C` | Carry; on `CMP`/`CPX`/`CPY` it means *register >= operand*; on `DIV` it flags a divide by zero. |

`SEP #mask` sets bits and `REP #mask` clears them. `REP #$30` gives 16-bit `A`, `X` and `Y`; `SEP #$30` gives 8-bit.
`PHP` pushes `P`; `POPF` pulls it (there is no `PLP` opcode, but `cpuasm` accepts `PLP` as a spelling of `POPF`).

### What M and X change

- **Width of the operation.** `M` picks the width of `A`-based loads, stores, arithmetic and logic. `X` picks it for
  `LDX`/`LDY`/`STX`/`STY`, `CPX`/`CPY`, `INX`/`INY`/`DEX`/`DEY` and for adding an index to an address.
- **Immediate operands do not change size.** A `#const` operand is always two bytes in the encoding (the instruction is 3
  bytes) whatever `M` and `X` are. In an 8-bit mode the low byte is used and the high byte is ignored. This is unlike the
  65C816, where `LDA #` is 2 or 3 bytes depending on `M`, and it means the same code assembles identically in either mode.
- **Cycle counts do not change.** See [timing.md](timing.md).
- With `M=1`, writes to `A` leave its high byte as it was. With `X=1`, loads, increments, shifts and transfers into `X`/`Y` clear the high byte (`DIV` is the exception: it keeps it).

## Differences from the 65C816

If you know the 65816, these are the ones that matter.

| | 65C816 | DEF88186 |
|---|--------|----------|
| Modes | Emulation and native, `XCE` switches | Native only. No `E` flag, no `XCE`, no `XBA`. |
| Reset | Reads the reset vector | Starts at `$00:0000` or the Boot ROM; no vector |
| Direct page | Bank `$00` RAM | Bank `$00`, which is **cartridge ROM**. Reads work, writes are dropped. |
| Stack | Bank `$00`, page 1 in emulation mode | **Bank `$80`** (work RAM); `SP` is a full 16 bits and wraps inside the bank |
| Branches | Eight relative branches | Six branches with **absolute** targets: `BMI BCS BEQ` (24-bit), `BVS BRA` (16-bit), `BRL` (relative). `BNE BPL BCC BVC` are assembler pseudo-instructions. |
| `JMP abs` | Stays in `PB` | Goes to bank **`DB`** |
| Long call | `JSL`/`RTL` | `CALL`/`RTL`, which resume one byte late; see [instructions.md](instructions.md) |
| Block move | `MVN` ascends, `MVP` descends; machine code stores destination bank first | `MVN` **descends**, `MVP` **ascends**; source bank first |
| Pulls | `PLA PLX PLY PLB PLD PLP` | `PLA` and `POPF` only |
| Stack transfers | `TXS`, `TSX`, `TCS`, `TSC` | `TCS`, `TSC` only |
| `CPX`/`CPY` | Immediate, direct page, absolute | **Absolute only** |
| `BIT #` | Sets only `Z` | Sets `N`, `V` and `Z` like the other forms |
| Interrupt frame | `PB`, `PC`, `P` in native mode | The same four bytes, but only the native vectors exist |
| Timing | A cycle per bus access | Fixed per-instruction costs; fetches are free |
| New instructions | | `MUL`, `DIV`, `LOOP`/`LPEND`, `SHL`/`SHR`/`RCL`, `PUSH #`, `CALL`/`RET`, `SDB`, `HLT` |

## The 8086 side

The hardware multiplier, divider, loop counter and `SHL`/`SHR`/`RCL` are the 8086-flavoured additions. They behave like
ordinary instructions; their details (and their quirks) are in [instructions.md](instructions.md).

## Clock

The CPU runs at master / 4 = 17.002839 MHz, one cycle every 58.8 ns, which is 4 master cycles. It shares the master clock
with the PPU (master / 1), the DMA controller (master / 2) and the APU (master / 16, or / 8 in its fast mode).
