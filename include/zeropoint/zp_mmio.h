/*
 * zp_mmio.h - ZeroPoint memory map and memory-mapped I/O register addresses.
 *
 * These mirror the emulator (see src/cpu.cpp, CLAUDE.md "Memory Maps").
 * All addresses are full 24-bit CPU bus addresses.
 *
 * Because the DEF88186 has a 16-bit native word but a 24-bit bus, C code
 * cannot dereference these directly with a plain pointer.  Use the peek/poke
 * helpers below (implemented in libzp with absolute-long addressing) or the
 * higher-level helpers in zp_dma.h / zp_ppu.h / zp_apu.h / zp_timer.h.
 */
#ifndef ZEROPOINT_ZP_MMIO_H
#define ZEROPOINT_ZP_MMIO_H

#include "zp_types.h"

/* ---- Top-level 24-bit memory map (bank granularity) -------------------- */
#define ZP_ROM_BASE     0x000000UL   /* $00-$7F : ROM        (8 MB)  */
#define ZP_RAM_BASE     0x800000UL   /* $80-$9F : RAM        (2 MB)  */
#define ZP_APU_WIN_BASE 0xA00000UL   /* $A0     : APU window (64 KB) */
#define ZP_PPU_WIN_BASE 0xB00000UL   /* $B0     : PPU window (64 KB) */
#define ZP_IO_BASE      0xD80000UL   /* $D8     : I/O registers      */
#define ZP_BOOT_BASE    0xFF0000UL   /* $FF     : Boot ROM   (64 KB) */

/* ---- I/O register blocks (bank $D8) ------------------------------------ */
#define ZP_IO_PPU       (ZP_IO_BASE + 0x0000UL)  /* PPU control   ($D80000) */
#define ZP_IO_APU       (ZP_IO_BASE + 0x0010UL)  /* APU control   ($D80010) */
#define ZP_IO_DMA       (ZP_IO_BASE + 0x0020UL)  /* DMA control   ($D80020) */
#define ZP_IO_PORTS     (ZP_IO_BASE + 0x0030UL)  /* Player/debug  ($D80030) */
#define ZP_IO_DISPLAY   (ZP_IO_BASE + 0x0040UL)  /* Display status($D80040) */
#define ZP_IO_SYSCTL    (ZP_IO_BASE + 0x0048UL)  /* System control($D80048) */
#define ZP_IO_TIMER     (ZP_IO_BASE + 0x0050UL)  /* Timer control ($D80050) */

/* ---- PPU control block ($D80000-$D8000F) ------------------------------- */
#define ZP_PPU_CTRL     (ZP_IO_PPU + 0x00)  /* W: bit0 start, bit1 reset     */
#define ZP_PPU_STATUS   (ZP_IO_PPU + 0x01)  /* R: PPU state                  */
#define ZP_PPU_PC_LO    (ZP_IO_PPU + 0x02)
#define ZP_PPU_PC_HI    (ZP_IO_PPU + 0x03)
#define ZP_PPU_VBL_LO   (ZP_IO_PPU + 0x0B)  /* R59 V-blank interrupt vector  */
#define ZP_PPU_VBL_HI   (ZP_IO_PPU + 0x0C)
#define ZP_PPU_HBL_LO   (ZP_IO_PPU + 0x0D)  /* R60 H-blank interrupt vector  */
#define ZP_PPU_HBL_HI   (ZP_IO_PPU + 0x0E)
#define ZP_PPU_CTRL_START 0x01
#define ZP_PPU_CTRL_RESET 0x02

/* ---- APU control block ($D80010-$D8001F) ------------------------------- */
#define ZP_APU_CTRL     (ZP_IO_APU + 0x00)  /* W: b0 reset, b1 halt, b2 run  */
#define ZP_APU_STATUS   (ZP_IO_APU + 0x01)  /* R: bit0 halted                */
#define ZP_APU_PC_LO    (ZP_IO_APU + 0x02)
#define ZP_APU_PC_HI    (ZP_IO_APU + 0x03)
#define ZP_APU_SP_LO    (ZP_IO_APU + 0x04)
#define ZP_APU_SP_HI    (ZP_IO_APU + 0x05)
#define ZP_APU_ROMBANK  (ZP_IO_APU + 0x06)  /* W: select banked AROM         */
#define ZP_APU_IOBANK   (ZP_IO_APU + 0x07)
#define ZP_APU_CTRL_RESET 0x01
#define ZP_APU_CTRL_HALT  0x02
#define ZP_APU_CTRL_RUN   0x04

/* ---- DMA control block ($D80020-$D8002F) ------------------------------- */
/* Write 0 to RESET, then stream the 9-byte config to CONFIG (see zp_dma.h). */
#define ZP_DMA_RESET    (ZP_IO_DMA + 0x00)  /* W any: reset config index     */
#define ZP_DMA_CONFIG   (ZP_IO_DMA + 0x01)  /* W x9 : 9-byte config, auto-fire*/
#define ZP_DMA_STATUS   (ZP_IO_DMA + 0x01)  /* R: (queued<<4)|active         */
#define ZP_DMA_IRQ      (ZP_IO_DMA + 0x02)  /* R: 1 if paused by interrupt   */
#define ZP_DMA_CHAN(n)  (ZP_IO_DMA + 0x03 + (n)) /* R: channel n state (0-11) */

/* ---- Runtime-provided bus accessors (libzp: zp_mmio.asm) --------------- */
/* These use DEF88186 absolute-long addressing to reach any 24-bit address.
 * On def88186cc (no prototypes) the calls resolve via implicit declaration
 * and the definitions are supplied at link time by lib/zp_mmio.asm. */
#if ZP_HAS_PROTOTYPES
extern zpu8 zp_peek8(zpaddr addr);
extern void zp_poke8(zpaddr addr, zpu8 value);
#endif

/* 16-bit little-endian convenience wrappers. */
ZP_INLINE zpu16 zp_peek16(zpaddr a)
{
    return (zpu16)(zp_peek8(a) | ((zpu16)zp_peek8(a + 1) << 8));
}
ZP_INLINE void zp_poke16(zpaddr a, zpu16 v)
{
    zp_poke8(a, ZP_LO8(v));
    zp_poke8(a + 1, ZP_HI8(v));
}

#endif /* ZEROPOINT_ZP_MMIO_H */
