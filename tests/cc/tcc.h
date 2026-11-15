/* tcc.h - shared prelude for the compiler regression tests.
 *
 * Each test is an ordinary C program that calls rec(v) / rec32(v) to record results. run.sh compiles it three ways
 * and compares the recorded lists: with the host gcc (-DHOST, the reference), with zpcc and with def88186cc
 * (-DZP_CC_DOS), the last two run in the emulator. Keep tests inside 16-bit int / 32-bit long range with no
 * overflow, so the host's wider int gives the same answers.
 *
 * Types are #defines (def88186cc can't use a typedef name as a type) and rec/rec32 are only declared for the
 * compilers that take prototypes (def88186cc resolves them implicitly).
 */
#ifndef TCC_H
#define TCC_H

#ifdef HOST
#include <stdio.h>
#define S16 short
#define U16 unsigned short
#define S32 int
#define U32 unsigned int
static void rec(S16 v) { printf("%d ", (int)v); }
static void rec32(U32 v) { printf("%d %d ", (int)(short)(v & 0xFFFF), (int)(short)(v >> 16)); }
#else
#define S16 int
#define U16 unsigned int
#define S32 long
#define U32 unsigned long
#ifndef ZP_CC_DOS
extern void rec(S16 v);
extern void rec32(U32 v);    /* records the low word, then the high word */
#endif
#endif

#endif
