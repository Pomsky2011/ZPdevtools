# Sample storage: SST and STL

Sample data lives in two tables in APU memory, both writable RAM:

| Range | Name | What |
|-------|------|------|
| `$9000-$CFFF` | SST | The sample data itself, in 16-byte blocks: 1,024 blocks, **12,288 samples** in all |
| `$7000-$7FFF` | STL | A table of 4-byte entries, one per sample: where it starts and where it loops |

12,288 8-bit samples is 256 ms at the native 48 kHz rate. That is why a real sound is stored at a low sample rate and
played back with a raised pitch, and why long audio uses the [stream port](mmp.md) or the [ADPCM player](adpcm.md)
instead.

A channel plays the sample whose STL entry address is in its `STL address` register ([mmp.md](mmp.md)).

## SST blocks

A block is 16 bytes: a 4-byte header, then 12 samples. Samples are **signed 8-bit PCM**.

```text
byte:   0    1    2    3   | 4 ... 15
        XX   YL   VU   TS  | sample 0 ... sample 11
```

Only two things in the header do anything:

| Header byte 1 | Meaning |
|---------------|---------|
| bit 2 (`$04`), **W** | this is the **final block** of the sample |
| bit 3 (`$08`), **Z** | on the final block: play to the end **once**, then stop (a one-shot) |

So a final looping block has byte 1 = `$04`, a final one-shot block `$0C`, and every earlier block `$00`. The rest of byte 1
(the `Y` nibble, bits 7-4, and bits 1-0) and bytes 0, 2 and 3 are defined in the format (a loop count `XX`, a loop start
inside the block `Y`, and two "clamping" adjustments `V U T S` meant to widen the dynamic range) but **nothing reads them**.
Write zeros.

Z is honoured only on the final block.

## Sectors

A **sector** is a run of blocks that make up one sample. The mixer copies blocks from the STL entry's start address,
one after another, until it has copied a block with W set. If no block ever sets W, it copies up to the end of SST, so
always set W on the last block.

The copy is made when a channel starts. Editing the blocks later does not affect a channel that is already playing; see
[mmp.md](mmp.md) for retriggering.

## STL entries

```text
byte:   0  1      2  3
        start     loop
```

| Field | |
|-------|---|
| `start` (16-bit) | address in SST of the sample's first block, `$9000-$CFFF` |
| `loop` (16-bit) | the **sample number** (not a byte address) to wrap to at the end |

`loop = 0` loops the whole sample. A loop point at or past the end counts as 0. To play once instead, set Z on the final
block. A channel's STL address must be inside `$7000-$7FFF`; entry `n` is at `$7000 + 4n`.

## Worked layouts

A one-block looping tone. SST at `$9000`, STL entry at `$7000`:

```text
$9000: 00 04 00 00   00 1E 3A 4F 5F 68 6A 64 57 44 2D 14      ; W=1, loops (Z=0); 12 sine samples
$7000: 00 90  00 00                                            ; start $9000, loop 0 = the whole block
```

An attack followed by a sustained loop, three blocks, 36 samples:

```text
$9100: 00 00 00 00  <12 attack samples>
$9110: 00 00 00 00  <12 decay samples>
$9120: 00 04 00 00  <12 sustain samples>                       ; final block, loops
$7010: 00 91  18 00                                            ; start $9100, loop to sample 24 ($18)
```

The channel plays samples 0-23 once, then loops 24-35 for as long as it stays on.

Because a sector wraps at its end and blocks hold 12 samples, a looping tone's length should be a multiple of 12.

## Making sample data

- `scripts/zpsnd.py` bakes sine, saw, square, noise and WAV samples, with loop points and crossfades, into a ready-made STL
  and SST image ([zpsnd.md](zpsnd.md)).
- `wav2mmp` (`executables/wav2mmp`, `scripts/wav2mmp.py`) converts a WAV to blocks but is **out of date**: the assembly it writes uses
  `SBF`, which no longer exists, and its usage comments recommend `WRH`/`WRL`, which are not implemented. Its output does not
  assemble. Use `zpsnd.py`.
- `examples/apu/play_sample.asm` writes a block and an STL entry by hand and starts a channel, and is the shortest complete
  example.
