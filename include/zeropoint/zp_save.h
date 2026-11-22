/*
 * zp_save.h - the cartridge's battery-backed save RAM (banks $C0-$C7).
 *
 * Build the ROM with `zpbuild game.elf -o game.zpb --save-size 8K` (a power of two, 1K..512K; the size is in the signed header)
 * and link lib/zp_save.asm next to zp_mmio.asm. The console then maps that many bytes at $C0:0000, mirrored across the 512 KB
 * window; they keep their value when the console is off (the emulator keeps them in game.sav). A fresh cartridge reads $FF
 * everywhere. A cartridge built without --save-size has nothing there: reads are open bus ($FF), writes go nowhere.
 * See the emulator's docs/cart-save.md.
 *
 * Offsets below are bytes from the start of the save RAM (0 .. size-1). zp_peek8/zp_poke8 do NOT reach these banks (they
 * keep every other program's timing exactly): use the zp_save_* calls.
 *
 * Power can go away in the middle of any write, so a save that matters goes through zp_save_store(): two slots, each
 * with a sequence number and a CRC, the older one rewritten, committed last. After a cut one slot is always intact.
 *
 *     zpu8 data[4];                                         (a local array: the compilers take its address as a RAM pointer)
 *     if (!zp_save_load(0UL, 32, data, 4)) { ...defaults... }   (nothing valid yet: a new cartridge, or a wiped one)
 *     ...
 *     zp_save_store(0UL, 32, data, 4);                      (slots of 32 bytes at offsets 0 and 32; each needs len + 6)
 */
#ifndef ZEROPOINT_ZP_SAVE_H
#define ZEROPOINT_ZP_SAVE_H

#include "zp_types.h"
#include "zp_mmio.h"

#define ZP_SAVE_BASE    0xC00000UL   /* $C0-$C7: battery save RAM */
#define ZP_SAVE_WINDOW  0x80000UL    /* 512 KB window; the cartridge's size repeats inside it */

/* Runtime-provided accessors (libzp: zp_save.asm), same convention as zp_peek8/zp_poke8. */
#if ZP_HAS_PROTOTYPES
extern zpu8 zp_sram_peek8(zpaddr addr);
extern void zp_sram_poke8(zpaddr addr, zpu8 value);
#endif

ZP_INLINE zpu8 zp_save_read8(zpu32 off)
{
    return zp_sram_peek8(ZP_SAVE_BASE + off);
}

ZP_INLINE void zp_save_write8(zpu32 off, zpu8 value)
{
    zp_sram_poke8(ZP_SAVE_BASE + off, value);
}

/* Copy len bytes between the save RAM and a RAM buffer. */
ZP_INLINE void zp_save_read(zpu32 off, zpu8 *dst, zpu16 len)
{
    zpu16 i;
    for (i = 0; i < len; i++) dst[i] = zp_save_read8(off + i);
}

ZP_INLINE void zp_save_write(zpu32 off, zpu8 *src, zpu16 len)
{
    zpu16 i;
    for (i = 0; i < len; i++) zp_save_write8(off + i, src[i]);
}

/* Set n bytes back to the erased value $FF (a wipe: zp_save_load() then finds nothing). */
ZP_INLINE void zp_save_erase(zpu32 off, zpu32 n)
{
    zpu32 i;
    for (i = 0; i < n; i++) zp_save_write8(off + i, 0xFF);
}

/* CRC-16/CCITT (poly $1021) step. */
ZP_INLINE zpu16 zp_save_crc(zpu16 crc, zpu8 b)
{
    int i;
    crc = (zpu16)(crc ^ ((zpu16)b << 8));
    for (i = 0; i < 8; i++) {
        if (crc & 0x8000) crc = (zpu16)((crc << 1) ^ 0x1021);
        else crc = (zpu16)(crc << 1);
    }
    return crc;
}

/* A slot is 6 header bytes + the data: [0] $A5 (written last: the commit), [1] sequence, [2..3] length, [4..5] CRC of
 * the sequence byte and the data. Returns the slot's sequence number (0..255), or -1 if it is not a complete slot of this length. */
ZP_INLINE int zp_save_slot_seq(zpu32 off, zpu16 len)
{
    zpu16 crc, i;
    if (zp_save_read8(off) != 0xA5) return -1;
    if ((zpu16)(zp_save_read8(off + 2) | ((zpu16)zp_save_read8(off + 3) << 8)) != len) return -1;
    crc = zp_save_crc(0xFFFF, zp_save_read8(off + 1));
    for (i = 0; i < len; i++) crc = zp_save_crc(crc, zp_save_read8(off + 6 + i));
    if ((zpu16)(zp_save_read8(off + 4) | ((zpu16)zp_save_read8(off + 5) << 8)) != crc) return -1;
    return (int)zp_save_read8(off + 1);
}

/* 1 if sequence b is newer than a (the counter wraps: newer means 1..127 ahead). */
ZP_INLINE int zp_save_newer(int b, int a)
{
    int d;
    d = (b - a) & 0xFF;
    return d != 0 && d < 128;
}

/* Load the newest intact slot of the pair at base and base + slot_size into buf. Returns 1, or 0 if neither is intact. */
ZP_INLINE zpbool zp_save_load(zpu32 base, zpu16 slot_size, zpu8 *buf, zpu16 len)
{
    int a, b;
    zpu32 off;
    a = zp_save_slot_seq(base, len);
    b = zp_save_slot_seq(base + slot_size, len);
    if (a < 0 && b < 0) return 0;
    if (a < 0) off = base + slot_size;
    else if (b < 0) off = base;
    else if (zp_save_newer(b, a)) off = base + slot_size;
    else off = base;
    zp_save_read(off + 6, buf, len);
    return 1;
}

/* Which slot zp_save_store() rewrites next, and with what sequence number: returns (seq << 1) | slot, slot 0 = A, 1 = B. It is the one that
 * is not the newest intact slot (an invalid slot first of all), so the newest state is never the one being overwritten. */
ZP_INLINE int zp_save_next(zpu32 base, zpu16 slot_size, zpu16 len)
{
    int a, b;
    a = zp_save_slot_seq(base, len);
    b = zp_save_slot_seq(base + slot_size, len);
    if (a < 0) return (((b < 0) ? 1 : ((b + 1) & 0xFF)) << 1);
    if (b < 0) return ((((a + 1) & 0xFF) << 1) | 1);
    if (zp_save_newer(b, a)) return ((((b + 1) & 0xFF) << 1));
    return ((((a + 1) & 0xFF) << 1) | 1);
}

/* A 16-bit little-endian store. */
ZP_INLINE void zp_save_write16(zpu32 off, zpu16 v)
{
    zp_save_write8(off, (zpu8)(v & 0xFF));
    zp_save_write8(off + 1, (zpu8)(v >> 8));
}

/* Write one slot: invalid while it is rewritten, data and header, the $A5 commit byte last. */
ZP_INLINE void zp_save_put(zpu32 off, zpu8 seq, zpu8 *buf, zpu16 len)
{
    zpu16 crc, i;
    zp_save_write8(off, 0x00);
    crc = zp_save_crc(0xFFFF, seq);
    for (i = 0; i < len; i++) {
        zp_save_write8(off + 6 + i, buf[i]);
        crc = zp_save_crc(crc, buf[i]);
    }
    zp_save_write8(off + 1, seq);
    zp_save_write16(off + 2, len);
    zp_save_write16(off + 4, crc);
    zp_save_write8(off, 0xA5);
}

/* Store buf as the newest state (see zp_save_next()). */
ZP_INLINE void zp_save_store(zpu32 base, zpu16 slot_size, zpu8 *buf, zpu16 len)
{
    int n;
    n = zp_save_next(base, slot_size, len);
    if (n & 1) zp_save_put(base + slot_size, (zpu8)(n >> 1), buf, len);
    else zp_save_put(base, (zpu8)(n >> 1), buf, len);
}

#endif /* ZEROPOINT_ZP_SAVE_H */
