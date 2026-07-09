/*
 * main.c - The same demo written in C against the ZeroPoint SDK.
 *
 * Compiles with BOTH toolchains:
 *   zpcc:       zpcc      -I../../include            main.c -o main.asm
 *   def88186cc: def88186cc -DZP_CC_DOS -I../../include main.c -o main.asm
 * then: cpuasm main.asm main.bin  (prepend crt0 / link with zplink).
 *
 * The DMA loaders, PPU/APU upload helpers, and timer API all come from the
 * umbrella header - no direct register pokes required.
 */
#include <zeropoint/zeropoint.h>

/* def88186cc has no '(void)' parameter list, so write main as main(). */
int main()
{
    /* Bring the PPU up with microcode DMA'd from ROM $100000. */
    zp_ppu_boot(0x100000UL, 2048, 0);

    /* Bring the APU up with a program DMA'd from ROM $200000. */
    zp_apu_boot(0x200000UL, 1024, 1);

    /* Run a 60 Hz main loop off the V-blank timer. */
    zp_timer_enable(ZP_TMR_VBLANK);
    for (;;) {
        zp_timer_wait(ZP_TMR_VBLANK);
        /* ... per-frame game logic ... */
    }
    /* not reached */
}
