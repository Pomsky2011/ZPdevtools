/* EXPECT: 255 90 90 90 0 1 1 4 1 -1 2 9 2 3 11 -1 9 1 21 22 1 0 0 10673 0 255 */
/* SAVE: 8192 */
/* The cartridge's battery save RAM (zp_save.h) on the real bus, through both compilers: blank reads, mirroring across the window, and the
 * two-slot checksummed store surviving a corrupted slot and a write cut short. 8 KB of SRAM on a dummy cartridge (run.sh: SAVE). */
#include <zeropoint/zp_types.h>
#include <zeropoint/zp_mmio.h>
#include <zeropoint/zp_save.h>
#include "tcc.h"

int main()
{
    zpu8 buf[4];
    zpu8 out[4];
    zpu8 s[9];
    zpu16 i, crc;
    int ok;

    rec(zp_save_read8(0UL));                    /* 255: a new cartridge is blank */
    zp_save_write8(10UL, 0x5A);
    rec(zp_save_read8(10UL));                   /* 90 */
    rec(zp_save_read8(10UL + 8192UL));          /* 90: 8 KB repeats across the window */
    rec(zp_save_read8(0x70000UL + 10UL));       /* 90: ...up to bank $C7 */

    ok = zp_save_load(64UL, 16, out, 4);
    rec(ok);                                    /* 0: nothing stored yet */
    for (i = 0; i < 4; i++) buf[i] = (zpu8)(i + 1);
    zp_save_store(64UL, 16, buf, 4);            /* slot A, sequence 1 */
    ok = zp_save_load(64UL, 16, out, 4);
    rec(ok);                                    /* 1 */
    rec(out[0]);                                /* 1 */
    rec(out[3]);                                /* 4 */
    rec(zp_save_slot_seq(64UL, 4));             /* 1 */
    rec(zp_save_slot_seq(80UL, 4));             /* -1: slot B is still erased */

    buf[0] = 9;
    zp_save_store(64UL, 16, buf, 4);            /* slot B, sequence 2 */
    rec(zp_save_slot_seq(80UL, 4));             /* 2 */
    ok = zp_save_load(64UL, 16, out, 4);
    rec(out[0]);                                /* 9 */
    rec(out[1]);                                /* 2 */

    buf[0] = 11;
    zp_save_store(64UL, 16, buf, 4);            /* slot A again, sequence 3 */
    rec(zp_save_slot_seq(64UL, 4));             /* 3 */
    ok = zp_save_load(64UL, 16, out, 4);
    rec(out[0]);                                /* 11 */

    zp_save_write8(64UL + 6, 0x77);             /* a bad byte in the newest slot: its CRC no longer matches */
    rec(zp_save_slot_seq(64UL, 4));             /* -1 */
    ok = zp_save_load(64UL, 16, out, 4);
    rec(out[0]);                                /* 9: the previous state */
    rec(ok);                                    /* 1 */

    buf[0] = 21;
    zp_save_store(64UL, 16, buf, 4);            /* A is invalid, so it is rewritten: sequence 3 */
    zp_save_write8(80UL, 0);                    /* power cut while writing B: its commit byte is gone... */
    zp_save_write8(86UL, 0x55);                 /* ...and its data is half there */
    ok = zp_save_load(64UL, 16, out, 4);
    rec(out[0]);                                /* 21: A is whole */
    buf[0] = 22;
    zp_save_store(64UL, 16, buf, 4);            /* B again, sequence 4 */
    ok = zp_save_load(64UL, 16, out, 4);
    rec(out[0]);                                /* 22 */

    rec(zp_save_newer(1, 255));                 /* 1: the counter wrapped */
    rec(zp_save_newer(255, 1));                 /* 0 */
    rec(zp_save_newer(5, 5));                   /* 0 */

    for (i = 0; i < 9; i++) s[i] = (zpu8)('1' + i);
    crc = 0xFFFF;
    for (i = 0; i < 9; i++) crc = zp_save_crc(crc, s[i]);
    rec((S16)crc);                              /* 10673 = $29B1, the CRC-16/CCITT-FALSE check value of "123456789" */

    zp_save_erase(64UL, 32UL);
    ok = zp_save_load(64UL, 16, out, 4);
    rec(ok);                                    /* 0 */
    rec(zp_save_read8(64UL));                   /* 255 */
    return 0;
}
