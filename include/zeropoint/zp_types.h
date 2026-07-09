/*
 * zp_types.h - Fixed-width scalar types for the ZeroPoint fantasy console.
 *
 * Deliberately dependency-free so the same header compiles under:
 *   - zpcc      (clang/LLVM backend, modern C)
 *   - def88186cc (Turbo C 2.0 / MS-DOS 4.01+, strict C89 - no <stdint.h>)
 *
 * The DEF88186 is a 16-bit CPU with a 24-bit address bus.  Both target
 * compilers use a 16-bit int and a 32-bit long, so the widths below are
 * stable across the whole toolchain.
 */
#ifndef ZEROPOINT_ZP_TYPES_H
#define ZEROPOINT_ZP_TYPES_H

/*
 * Fixed-width types.
 *
 * def88186cc parses 'typedef' declarations but cannot yet use a typedef name
 * as a type in a later declaration (its own test_typedef.c documents this).
 * So in ZP_CC_DOS mode the widths are #define'd to built-in keyword spellings
 * (which def88186cc handles); every other compiler gets real typedefs.
 */
#ifdef ZP_CC_DOS
#  define zpu8   unsigned char
#  define zps8   signed char
#  define zpu16  unsigned int
#  define zps16  signed int
#  define zpu32  unsigned long
#  define zps32  signed long
#  define zpbool unsigned char
#  define zpaddr unsigned long   /* a 24-bit bus address rides in 32 bits */
#else
typedef unsigned char  zpu8;    /* 8-bit  unsigned */
typedef signed   char  zps8;    /* 8-bit  signed   */
typedef unsigned int   zpu16;   /* 16-bit unsigned (native word) */
typedef signed   int   zps16;   /* 16-bit signed   */
typedef unsigned long  zpu32;   /* 32-bit unsigned (holds a 24-bit address) */
typedef signed   long  zps32;   /* 32-bit signed   */
typedef zpu8           zpbool;
typedef zpu32          zpaddr;  /* a 24-bit CPU bus address rides in 32 bits */
#endif

#define ZP_TRUE  1
#define ZP_FALSE 0

/*
 * Compiler adaptation.
 *
 * The DOS compiler def88186cc accepts a small C subset: no 'static', no
 * function prototypes, and no '(void)' parameter lists.  Define ZP_CC_DOS
 * (the def88186cc build sets it automatically below) to compile the SDK in
 * that dialect - helpers become ordinary single-translation-unit functions
 * and prototypes are dropped in favour of def88186cc's implicit declarations.
 *
 * zpcc (clang) and any hosted C compiler get the normal 'static' helpers and
 * real prototypes.
 */
#if defined(__DEF88186CC__) && !defined(ZP_CC_DOS)
#  define ZP_CC_DOS 1
#endif

#ifdef ZP_CC_DOS
#  define ZP_INLINE           /* no 'static': one translation unit per program */
#  define ZP_VOID             /* def88186cc has no '(void)' parameter list     */
#  define ZP_CONST            /* def88186cc has no 'const' qualifier           */
#  define ZP_HAS_PROTOTYPES 0
#else
#  if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#    define ZP_INLINE static inline
#  else
#    define ZP_INLINE static
#  endif
#  define ZP_VOID void
#  define ZP_CONST const
#  define ZP_HAS_PROTOTYPES 1
#endif

/* Compose / decompose a 24-bit bank:offset address. */
#define ZP_ADDR(bank, off)  (((zpu32)(zpu8)(bank) << 16) | ((zpu16)(off)))
#define ZP_BANK_OF(a)       ((zpu8)(((zpu32)(a) >> 16) & 0xFFUL))
#define ZP_OFF_OF(a)        ((zpu16)((zpu32)(a) & 0xFFFFUL))

/* Little-endian byte extraction (the DEF88186 is little-endian). */
#define ZP_LO8(v)   ((zpu8)((v) & 0xFF))
#define ZP_HI8(v)   ((zpu8)(((v) >> 8) & 0xFF))
#define ZP_B0(v)    ((zpu8)((v) & 0xFF))
#define ZP_B1(v)    ((zpu8)(((v) >> 8) & 0xFF))
#define ZP_B2(v)    ((zpu8)(((v) >> 16) & 0xFF))

#endif /* ZEROPOINT_ZP_TYPES_H */
