/*
 * zp_apu.h - APU window access and DMA-based program/sample upload.
 *
 * The APU (4 MHz, 8-bit RISC) has 64 KB + 448 KB banked AROM, reached from the
 * CPU through the bank $A0 window (ZP_APU_WIN_BASE).  Upload APU programs and
 * SST sample data by DMA, select the AROM bank via ZP_APU_ROMBANK, then run.
 */
#ifndef ZEROPOINT_ZP_APU_H
#define ZEROPOINT_ZP_APU_H

#include "zp_types.h"
#include "zp_mmio.h"
#include "zp_dma.h"

/* CPU address of an APU-internal offset. */
#define ZP_APU(off)       (ZP_APU_WIN_BASE + (zpu16)(off))

#define ZP_APU_CODE       0x0000   /* APU program load base            */
#define ZP_APU_SST        0x8000   /* conventional sample-store base   */
#define ZP_APU_EXIO       0x2000   /* extended I/O (APU EXIO_BASE)     */

/* Core control via the $D8 APU control block. */
ZP_INLINE void zp_apu_reset(ZP_VOID)  { zp_poke8(ZP_APU_CTRL, ZP_APU_CTRL_RESET); }
ZP_INLINE void zp_apu_halt(ZP_VOID)   { zp_poke8(ZP_APU_CTRL, ZP_APU_CTRL_HALT);  }
ZP_INLINE void zp_apu_run(ZP_VOID)    { zp_poke8(ZP_APU_CTRL, ZP_APU_CTRL_RUN);   }
ZP_INLINE void zp_apu_rombank(zpu8 bank) { zp_poke8(ZP_APU_ROMBANK, bank); }

ZP_INLINE void zp_apu_set_pc(zpu16 pc)
{
    zp_poke8(ZP_APU_PC_LO, ZP_LO8(pc));
    zp_poke8(ZP_APU_PC_HI, ZP_HI8(pc));
}

/* Upload a blob from CPU ROM/RAM into the APU window at apu_off. */
ZP_INLINE zpu16 zp_apu_upload(zpaddr src, zpu16 apu_off, zpu32 len, zpu8 chan)
{
    return zp_dma_load(src, ZP_APU(apu_off), len, chan);
}

/* Load an APU program, reset PC to 0, and start it. */
ZP_INLINE void zp_apu_boot(zpaddr code_src, zpu32 code_len, zpu8 chan)
{
    zp_apu_reset();
    zp_apu_upload(code_src, ZP_APU_CODE, code_len, chan);
    zp_apu_set_pc(0);
    zp_apu_run();
}

/* Upload sample data (SST) into the APU sample store. */
ZP_INLINE zpu16 zp_apu_upload_samples(zpaddr src, zpu32 len, zpu8 chan)
{
    return zp_apu_upload(src, ZP_APU_SST, len, chan);
}

#endif /* ZEROPOINT_ZP_APU_H */
