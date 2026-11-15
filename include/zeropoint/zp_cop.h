/*
 * zp_cop.h - Access to every COP signature: the generic zp_cop()/zp_cop32() and typed
 * wrappers for the Boot ROM services.
 *
 * A COP signature is an instruction operand ("COP #$F0"), so it cannot be a C argument on
 * its own: libzp's zp_cop.asm builds the instruction at run time and returns the service's
 * registers.  Everything below reduces to that one routine.  The Boot ROM services need the
 * real Boot ROM (--boot / System::loadBootROM()); with the default stub $F0-$FE are the
 * cartridge's own software interrupt ($00:FFE4).  See docs/boot-math-services.md,
 * docs/boot-handoff.md and zpb-format.md in the ZeroPoint repo.
 *
 * Link against libzp (lib/zp_cop.asm).  Not re-entrant: do not call from an IRQ handler
 * that can interrupt another zp_cop call.
 */
#ifndef ZEROPOINT_ZP_COP_H
#define ZEROPOINT_ZP_COP_H

#include "zp_types.h"

/* ---- Signatures ($F0-$FF are the Boot ROM's; $00-$EF go to the cartridge's vector) ---- */
#define ZP_COP_SIN          0xF0   /* A = angle              -> A = sin, Q1.15            */
#define ZP_COP_COS          0xF1   /* A = angle              -> A = cos, Q1.15            */
#define ZP_COP_SINCOS       0xF2   /* A = angle              -> A = sin, X = cos          */
#define ZP_COP_ATAN2        0xF3   /* X = x, Y = y (signed)  -> A = angle                 */
#define ZP_COP_ATAN         0xF4   /* A = ratio, Q8.8 signed -> A = angle in +-$4000      */
#define ZP_COP_SQRT         0xF5   /* A = low, X = high word -> A = floor(sqrt)           */
#define ZP_COP_HYPOT        0xF6   /* X = x, Y = y (signed)  -> A = floor(sqrt(x*x+y*y))  */
#define ZP_COP_PPU_LOAD     0xF7   /* A = bank, X = offset, Y = 256-byte pages -> A = 0   */
#define ZP_COP_RESERVED_LO  0xF8   /* $F8-$FE: return at once, registers untouched        */
#define ZP_COP_RESERVED_HI  0xFE
#define ZP_COP_VERIFY_CHUNK 0xFF   /* X = chunk index (v3 signed ROM data region)         */

/* Angles are unsigned, 65536 = one full turn, 0 = +x, $4000 = +y (counter-clockwise). */

/* ---- Runtime-provided (libzp: zp_cop.asm) ------------------------------------------- */
/* All of these are assembly (the register moves are explicit there).  On def88186cc (no
 * "extern", no prototypes in the SDK headers) an undeclared call is an int in A; the two
 * that return 32 bits (A:X) are declared further down so it reads both words. */
#if ZP_HAS_PROTOTYPES
/* Calls COP #sig with the service's A, X, Y = a, x, y; works for any signature, including
 * the cartridge's own ($00-$EF).  Returns A (low word) and X (high word); a COP that
 * returns nothing leaves them as it found them.  DB, D, SP and the caller's M/X/flags are
 * preserved (including across COP #$FF, which would clobber DB). */
extern zpu32 zp_cop32(zpu8 sig, zpu16 a, zpu16 x, zpu16 y);
extern zpu16 zp_cop(zpu8 sig, zpu16 a, zpu16 x, zpu16 y);   /* same call, just A */
/* Three registers, Y = 0: no stack argument.  Y is only needed by ATAN2/HYPOT/PPU_LOAD, which
 * have wrappers below; zp_cop/zp_cop32 take all of A, X and Y (the 4th argument is passed on
 * the stack). */
extern zpu16 zp_cop3(zpu8 sig, zpu16 a, zpu16 x);

/* Boot ROM services. */
extern zps16 zp_sin(zpu16 angle);
extern zps16 zp_cos(zpu16 angle);
extern zpu32 zp_sincos(zpu16 angle);              /* sin in the low word, cos in the high */
extern zpu16 zp_atan2(zps16 y, zps16 x);
extern zps16 zp_atan(zps16 ratio_q8_8);
extern zpu16 zp_sqrt(zpu16 lo, zpu16 hi);         /* floor(sqrt(hi:lo)) */
extern zpu16 zp_hypot(zps16 x, zps16 y);
/* Resets the PPU, copies `pages` * 256 bytes from bank:offset to PPU address 0 and starts it. */
extern void zp_boot_ppu_load(zpu8 bank, zpu16 offset, zpu16 pages);
/* Asks the Boot ROM to verify data chunk `index` (16384 bytes at codeSize + index*16384).
 * There is no result: an unverified chunk keeps reading as 0xDE/0xAF poison. */
extern void zp_verify_chunk(zpu16 index);
#else
/* def88186cc: prototypes without "extern" (it has no static/const/(void)), so the calls that return a 32-bit value
 * - result in A:X - know to read both words; everything else is an int in A and needs no declaration. */
zpu32 zp_cop32(zpu8 sig, zpu16 a, zpu16 x, zpu16 y);
zpu32 zp_sincos(zpu16 angle);
#endif

#define ZP_SINCOS_SIN(r)  ((zps16)((r) & 0xFFFFUL))
#define ZP_SINCOS_COS(r)  ((zps16)(((r) >> 16) & 0xFFFFUL))
#define zp_sqrt32(v)      zp_sqrt((zpu16)((v) & 0xFFFFUL), (zpu16)(((v) >> 16) & 0xFFFFUL))

#endif /* ZEROPOINT_ZP_COP_H */
