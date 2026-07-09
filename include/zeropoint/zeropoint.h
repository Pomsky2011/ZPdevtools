/*
 * zeropoint.h - Umbrella header for the ZeroPoint fantasy console C SDK.
 *
 * Include this one header to get the full memory map, MMIO register set, DMA
 * loaders, PPU/APU upload helpers, and hardware timer API.  Everything here is
 * strict C89 so it builds identically under zpcc (clang) and def88186cc (DOS).
 *
 *   #include <zeropoint/zeropoint.h>
 *
 * Link against libzp (crt0 + zp_mmio.asm) so zp_peek8/zp_poke8 resolve.
 */
#ifndef ZEROPOINT_H
#define ZEROPOINT_H

#define ZP_SDK_VERSION_MAJOR 1
#define ZP_SDK_VERSION_MINOR 0
#define ZP_SDK_VERSION       "1.0"

#include "zp_types.h"
#include "zp_mmio.h"
#include "zp_dma.h"
#include "zp_ppu.h"
#include "zp_apu.h"
#include "zp_timer.h"

#endif /* ZEROPOINT_H */
