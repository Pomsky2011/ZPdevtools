/*
 * compat.h - MS-DOS/C89 Compatibility Header
 *
 * Provides stdint.h types and other C99 features for older compilers
 * Compatible with:
 * - MS-DOS 4.01+ with Turbo C 2.0 / Microsoft C 6.0
 * - 80286 processors (16-bit real mode)
 * - 2 MB RAM minimum
 *
 * Usage: Include this instead of stdint.h in all dev tools
 */

#ifndef ZEROPOINT_COMPAT_H
#define ZEROPOINT_COMPAT_H

/* Detect compiler and platform */
#if defined(__TURBOC__) || defined(__BORLANDC__)
    #define COMPILER_TURBO_C
#elif defined(_MSC_VER) && _MSC_VER < 1300
    #define COMPILER_MS_C
#elif defined(__WATCOMC__)
    #define COMPILER_WATCOM
#endif

/* Include stdint.h if available, otherwise provide types */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    /* C99 or later - use standard header */
    #include <stdint.h>
#elif defined(__GNUC__) || defined(__clang__)
    /* Modern GCC/Clang in C89 mode - they have stdint.h anyway */
    #include <stdint.h>
#elif !defined(_STDINT_H_) && !defined(_STDINT_H) && !defined(_SYS_STDINT_H_)
    /* Old C89 compilers (Turbo C, MS C) - provide types manually */

    /* Exact-width integer types */
    typedef signed char        int8_t;
    typedef unsigned char      uint8_t;
    typedef short              int16_t;
    typedef unsigned short     uint16_t;
    typedef long               int32_t;
    typedef unsigned long      uint32_t;

    /* Minimum-width integer types */
    typedef signed char        int_least8_t;
    typedef unsigned char      uint_least8_t;
    typedef short              int_least16_t;
    typedef unsigned short     uint_least16_t;
    typedef long               int_least32_t;
    typedef unsigned long      uint_least32_t;

    /* Fastest minimum-width integer types */
    typedef int                int_fast8_t;
    typedef unsigned int       uint_fast8_t;
    typedef int                int_fast16_t;
    typedef unsigned int       uint_fast16_t;
    typedef long               int_fast32_t;
    typedef unsigned long      uint_fast32_t;

    /* Integer types capable of holding object pointers */
    #if defined(__LARGE__) || defined(__COMPACT__) || defined(__HUGE__)
        typedef long           intptr_t;
        typedef unsigned long  uintptr_t;
    #else
        typedef int            intptr_t;
        typedef unsigned int   uintptr_t;
    #endif

    /* Greatest-width integer types */
    typedef long               intmax_t;
    typedef unsigned long      uintmax_t;

    /* Limits of exact-width integer types */
    #define INT8_MIN           (-128)
    #define INT8_MAX           127
    #define UINT8_MAX          255
    #define INT16_MIN          (-32768)
    #define INT16_MAX          32767
    #define UINT16_MAX         65535U
    #define INT32_MIN          (-2147483647L - 1)
    #define INT32_MAX          2147483647L
    #define UINT32_MAX         4294967295UL
#endif

/* Boolean type for C89 */
#ifndef __cplusplus
    #if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
        typedef int bool;
        #define true  1
        #define false 0
    #else
        #include <stdbool.h>
    #endif
#endif

/* inline keyword for C89 */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    #if defined(COMPILER_TURBO_C) || defined(COMPILER_MS_C)
        #define inline
    #elif defined(__GNUC__)
        #define inline __inline__
    #else
        #define inline
    #endif
#endif

/* snprintf for C89 - not available in Turbo C, must use sprintf */
/* WARNING: When using sprintf, ensure buffer is large enough! */

/* File size limits for MS-DOS 4.01 */
#ifdef COMPILER_TURBO_C
    /* Turbo C uses 16-bit int, so file operations need care */
    #define MAX_FILE_SIZE 65535L
#else
    #define MAX_FILE_SIZE 4294967295UL
#endif

/* Memory allocation limits for 2 MB RAM systems */
#define MAX_REASONABLE_ALLOC 1048576L  /* 1 MB max allocation */

/* Path separators */
#if defined(__MSDOS__) || defined(_MSDOS) || defined(__DOS__)
    #define PATH_SEPARATOR '\\'
    #define PATH_SEPARATOR_STR "\\"
#else
    #define PATH_SEPARATOR '/'
    #define PATH_SEPARATOR_STR "/"
#endif

/* Safe memory allocation with size check */
#define SAFE_MALLOC(size) \
    ((size) > MAX_REASONABLE_ALLOC ? NULL : malloc(size))

#define SAFE_CALLOC(count, size) \
    ((count) * (size) > MAX_REASONABLE_ALLOC ? NULL : calloc(count, size))

#endif /* ZEROPOINT_COMPAT_H */
