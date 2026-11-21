/*
 * zp_pad.h - the controllers ($D80030-$D80032 for player 1, $D80038-$D8003A for player 2).
 *
 *   zp_pad_dir(0)    the direction: one of the ZP_DIR_* codes (diagonals are codes of their own), 0 = neutral
 *   zp_pad_ctrl(0)   the pad, bumpers, triggers and menu buttons (ZP_CTRL_*), ZP_CTRL_CONNECTED says it is plugged in
 *   zp_pad_btn(0)    the four face buttons in bits 7-4 (ZP_BTN_*)
 *   zp_pad_dx(0) / zp_pad_dy(0)   the direction as -1, 0 or 1 on each axis, whichever diagonal it is
 *
 * All the reads are live (no latching): read once per frame and keep the value if you need an edge.
 */
#ifndef ZEROPOINT_ZP_PAD_H
#define ZEROPOINT_ZP_PAD_H

#include "zp_types.h"
#include "zp_mmio.h"

#define ZP_PAD1_DIR  (ZP_IO_PORTS + 0x00)   /* $D80030 */
#define ZP_PAD1_CTRL (ZP_IO_PORTS + 0x01)   /* $D80031 */
#define ZP_PAD1_BTN  (ZP_IO_PORTS + 0x02)   /* $D80032 */
#define ZP_PAD2_DIR  (ZP_IO_PORTS + 0x08)   /* $D80038: reads 0 until a frontend plugs a second controller in */

#define ZP_DIR_UPLEFT    0x80
#define ZP_DIR_UP        0x40
#define ZP_DIR_UPRIGHT   0x20
#define ZP_DIR_RIGHT     0x10
#define ZP_DIR_DOWNRIGHT 0x08
#define ZP_DIR_DOWN      0x04
#define ZP_DIR_DOWNLEFT  0x02
#define ZP_DIR_LEFT      0x01

#define ZP_CTRL_PAD       0x80
#define ZP_CTRL_BUMPER_L  0x40
#define ZP_CTRL_BUMPER_R  0x20
#define ZP_CTRL_TRIGGER_L 0x10
#define ZP_CTRL_TRIGGER_R 0x08
#define ZP_CTRL_MENU      0x04
#define ZP_CTRL_PAUSE     0x02
#define ZP_CTRL_CONNECTED 0x01

#define ZP_BTN_4 0x80
#define ZP_BTN_3 0x40
#define ZP_BTN_2 0x20
#define ZP_BTN_1 0x10

/* player 0 or 1 */
ZP_INLINE zpu8 zp_pad_dir(int player)  { return zp_peek8((player ? ZP_PAD2_DIR : ZP_PAD1_DIR)); }
ZP_INLINE zpu8 zp_pad_ctrl(int player) { return zp_peek8((player ? ZP_PAD2_DIR : ZP_PAD1_DIR) + 1); }
ZP_INLINE zpu8 zp_pad_btn(int player)  { return zp_peek8((player ? ZP_PAD2_DIR : ZP_PAD1_DIR) + 2); }

ZP_INLINE int zp_pad_dx(int player)
{
    zpu8 d;
    d = zp_pad_dir(player);
    if (d & (ZP_DIR_RIGHT | ZP_DIR_UPRIGHT | ZP_DIR_DOWNRIGHT)) return 1;
    if (d & (ZP_DIR_LEFT | ZP_DIR_UPLEFT | ZP_DIR_DOWNLEFT)) return -1;
    return 0;
}
ZP_INLINE int zp_pad_dy(int player)
{
    zpu8 d;
    d = zp_pad_dir(player);
    if (d & (ZP_DIR_DOWN | ZP_DIR_DOWNLEFT | ZP_DIR_DOWNRIGHT)) return 1;
    if (d & (ZP_DIR_UP | ZP_DIR_UPLEFT | ZP_DIR_UPRIGHT)) return -1;
    return 0;
}

#endif /* ZEROPOINT_ZP_PAD_H */
