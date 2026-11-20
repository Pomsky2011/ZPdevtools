# MMP: the Music Mixing Processor

The MMP mixes 16 sample channels and one stereo stream into the console's 48 kHz stereo output. It is driven by registers in
the first 256 bytes of APU memory. The APU's own program, or the CPU through the APU window, writes them.

Each output tick the MMP advances every active channel by its pitch, reads the sample with cubic interpolation, scales it by
volume and pan, and adds the channels together. The sum is clamped to 16 bits.

## Registers

| Address | Register | Size | |
|---------|----------|------|---|
| `$0000 + 2n` | pitch, channel `n` | 16-bit | `$1000` = native speed |
| `$0020 + n` | volume | 8-bit | 0 silent, 255 full |
| `$0030 + n` | pan | 8-bit | 0 all left, 128 centre, 255 all right |
| `$0040 + n` | reserved | 8-bit | stored, no effect |
| `$0050 + 2n` | STL address | 16-bit | `0` = channel off; else a sample-table entry ([sst.md](sst.md)) |
| `$0070-$007F` | stream port | 16 bytes | see below |
| `$0080-$00FF` | reserved | | |

`n` is 0 to 15. Sixteen-bit registers are little-endian and take effect on the **high byte** write, so write the low byte
first. A channel's registers read back 0 until they have been written, even though the channels start at pitch `$1000`,
volume 255 and pan 128.

These are the registers a program writes with `STAX`: with `DP = $00`, `STAX` plus a register holding `2n` or `n` is a
register address.

### Starting and stopping a channel

- **Start:** write the channel's STL address. The write that completes it (the high byte) is what starts playback, from
  sample 0.
- **Retrigger:** writing the *same* address again does nothing. To restart a note, write `0` first, then the address.
- **Stop:** write `0`. A one-shot sample also stops by itself at its end, and clears the channel's STL address when it
  does, so writing the same address afterwards retriggers.
- **Change the sample:** write a different address; the channel restarts from the beginning.

The sample is copied into a cache when the channel starts. Overwriting the sample in SST under a playing channel changes
nothing until the channel is retriggered.

### Pitch

Pitch is a fixed-point rate with 12 fractional bits: how many samples the channel advances per 48 kHz output tick, times
4096. `$1000` plays one sample per tick, `$2000` is an octave up, `$0800` an octave down.

A sample stored at `R` Hz plays at its natural speed at pitch `R * 4096 / 48000`. A sample of `N` samples looping as a
tone sounds at `48000 * (pitch / 4096) / N` Hz.

## Mixing

For each active channel, in order:

1. Take the four samples around the playback position and interpolate between the middle two with a Catmull-Rom cubic
   (exact integer arithmetic, so the audio is bit-identical on every host).
2. Scale the 8-bit result to 16 bits (times 256).
3. Multiply by `volume / 255`.
4. Multiply by `(255 - pan) / 255` for the left output and `pan / 255` for the right.
5. Add into the mix.

At pan 128 each side gets about half the amplitude. When the playback position passes the end of the sample, a looping
channel wraps back to its loop point and a one-shot stops; the interpolation looks ahead along the same path, so a loop
seam is as smooth as the data and a one-shot holds its last sample instead of wrapping to the start.

The mix is the sum of all channels plus the stream port, clamped to -32768..32767. Sixteen loud channels clip.

## Stream port, `$0070-$007F`

For audio that doesn't fit in the 12,288 samples of SST (a whole song), the stream port plays a **stereo 8-bit PCM ring
buffer** that the CPU or DMA keeps filled. It is added on top of the 16 channels. A frame is two signed bytes, left then
right.

| Address | Register | |
|---------|----------|---|
| `$70` | `CTRL` | bit 0 enable; bit 1 reset (self-clearing: read and write pointers to 0, position and status cleared) |
| `$71` | `STATUS` | bit 0 underrun (sticky: write 1 to clear); bit 1 playing (read-only: the last tick had data) |
| `$72` | `BASE` | 16-bit: APU address of the ring. Must be below `$D000`. |
| `$74` | `SIZE` | 16-bit: ring length in **frames** (so `2 * SIZE` bytes). Set before reset or enable. |
| `$76` | `RDPTR` | 16-bit, read-only: the frame the mixer is at. CPU writes are ignored. |
| `$78` | `WRPTR` | 16-bit: one past the last frame written. Takes effect on the high-byte write. |
| `$7A` | `PITCH` | 16-bit: `$1000` = one stream frame per output tick; cubic-resampled |
| `$7C` | `VOL` | 0-255, default 255 |

The ring is full at `SIZE - 1` frames. Free space is `(RDPTR - WRPTR - 1) mod SIZE`. The mixer needs three frames ahead of
`RDPTR` for its cubic interpolation: with fewer it outputs silence, holds its position, and sets `UNDERRUN`.

To play: write `BASE` and `SIZE`, set `CTRL = $03` (enable and reset), fill the ring, advance `WRPTR`, and keep ahead of
`RDPTR`. The output tick runs at 48 kHz, so at `PITCH = $1000` the ring drains at 48,000 frames a second; a 44.1 kHz
stream uses `PITCH = 44100 * 4096 / 48000 = $0EB3`. The [ADPCM player](adpcm.md) is a complete example.
