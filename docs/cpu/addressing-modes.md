# Addressing modes

An operand names a memory location in one of the forms below. Which instructions have which forms is irregular; the
matrix at the end of [opcode-table.md](opcode-table.md) is the reference.

Terms: `PB`, `DB`, `D`, `SP` are the registers from [architecture.md](architecture.md). "Index" is `X` or `Y`; with the
`X` flag set only its low byte is added. Sums of 16 bits wrap inside the bank unless noted.

## The forms

| Syntax | Name | Bytes | Effective address |
|--------|------|-------|-------------------|
| `#const` | Immediate | 3 | the two bytes after the opcode (low byte only when `M` or `X` is 1) |
| `dp` | Direct page | 2 | `$00:(D + dp)` |
| `dp,X` `dp,Y` | Direct page indexed | 2 | `$00:(D + dp + index)` |
| `addr` | Absolute | 3 | `DB:addr` |
| `addr,X` `addr,Y` | Absolute indexed | 3 | `DB:(addr + index)`, the sum wraps inside the bank |
| `long` | Absolute long | 4 | `long`, a full 24-bit address |
| `long,X` `long,Y` | Absolute long indexed | 4 | `long + index`, carries into the bank |
| `(dp)` | Direct page indirect | 2 | the 16-bit pointer at `$00:(D+dp)`, in bank `DB` |
| `(dp,X)` | Direct page indexed indirect | 2 | the 16-bit pointer at `$00:(D+dp+X)`, in bank `DB` |
| `(dp),Y` | Direct page indirect indexed | 2 | `DB:(pointer + Y)`, where the pointer is at `$00:(D+dp)` |
| `[dp]` | Direct page indirect long | 2 | the 24-bit pointer at `$00:(D+dp)`, used as it is |
| `[dp],Y` | Direct page indirect long indexed | 2 | the 24-bit pointer at `$00:(D+dp)`, plus Y, carrying into the bank |
| `n,S` | Stack relative | 2 | `$80:(SP + n)` |
| `(n,S),Y` | Stack relative indirect indexed | 2 | `DB:(pointer + Y)`, the pointer at `$80:(SP+n)` |
| `srcbk,destbk` | Block move | 3 | two bank bytes for `MVN`/`MVP` |

There is no `dp,S,X`: `STA 0,S,X` assembles as `STA dp,X`, silently. `dp,Y` exists only for `LDX` and `STX`.

### Direct page is bank `$00`

This is the form most likely to hurt. `D + dp` is an offset into **bank `$00`**, which is cartridge ROM. A direct-page
read gets ROM bytes. A direct-page write is dropped. On the 65816 the direct page is RAM, so ported code that keeps
variables there silently does nothing. Use absolute addressing with `DB = $80` for variables and keep direct page for
constant tables you actually want in ROM.

The pointer forms `(dp)`, `[dp]` and the others read their pointer from bank `$00` too, so the pointer itself must be in
ROM. To follow a pointer you built at run time, store it in RAM and use `(n,S),Y`, or load an index and use `addr,X`.

### The data bank

Absolute forms take their bank from `DB`. `SDB #bank` sets it (there is no `PLB`). After `SDB #$D8`, `STA $0021` means
`$D8:0021`, but the assembler picks a direct-page encoding for any operand up to `$FF`, so you must write `STA !$21` to
get the absolute one; see the next section.

## How the assembler chooses

`cpuasm` picks the form from the operand's value:

| You write | You get |
|-----------|---------|
| `LDA $FF` | direct page |
| `LDA $100` up to `LDA $FFFF` | absolute |
| `LDA $10000` and above | long |
| `LDA label` | absolute |
| `LDA >label` | long: the `>` forces it |
| `LDA !$10` | absolute: the `!` forces it, even for a small operand |

Branches and `JMP long` always take a full address (`BEQ`, `BMI`, `BCS`: 24 bits; `BRA`, `BVS`: 16 bits).

## Jumps and calls

These do not follow the table above, and the differences are traps.

| Syntax | Target |
|--------|--------|
| `JMP addr` | **`DB:addr`**. The bank is `DB`, not `PB`. |
| `JMP long` / `JML long` | the 24-bit address |
| `JMP (addr)` | `PB:` the 16-bit pointer read from `$00:addr` |
| `JMP [addr]` | the 24-bit pointer read from `$00:addr` |
| `JMP (addr,X)` | `PB:` the pointer read from `PB:(addr+X)` |
| `JSR addr` | `PB:addr`; the return address (minus 1) is pushed, 2 bytes |
| `JSR (addr,X)` | as `JMP (addr,X)`, with the same push |
| `BRA addr`, `BVS addr` | `PB:addr` |
| `BMI long`, `BCS long`, `BEQ long` | the 24-bit address, and `PB` changes |
| `BRL label` | `PC` plus a signed 16-bit offset, within the bank |

`JMP addr` is the bad one. If `DB` differs from `PB` (say you did `SDB #$80` to reach RAM), `JMP loop` jumps into
bank `$80` and executes whatever is there. `JSR` explicitly ignores the `DB` bit it computes, but `JMP` does not. Use
`BRA label` for a jump inside the current bank, or `JMP long`.

`JMP (addr)` and `JMP [addr]` read their pointer from bank `$00` regardless of `PB`, while `JMP (addr,X)` reads from
`PB`. When `PB` is `$00` the difference does not show.

## Cycle effects

Reads through `addr,X`, `addr,Y` and `(dp),Y` cost one extra cycle when adding the index carries into the high byte of the
address (a page crossing). Stores and read-modify-write instructions through those forms always pay the cycle; it is
already in their cycle count in [opcode-table.md](opcode-table.md). Every other form costs the same however the index
falls. See [timing.md](timing.md).
