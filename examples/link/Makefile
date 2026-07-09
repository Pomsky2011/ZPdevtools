# ZeroPoint link example - builds the game as an ELF with zplink (dev side),
# then signs it into a .zpb ROM with zpbuild (the HQ mastering step).
#
#   make            # build the ELF and sign it into a .zpb
#   make elf        # dev-side link only -> game.elf
#   make rom        # sign the ELF -> game.zpb (uses the bundled dev key)
#   make clean

BIN     = ../../executables
CPUASM  = $(BIN)/cpuasm
ZPLINK  = $(BIN)/zplink
ZPBUILD = $(BIN)/zpbuild

all: rom

# --- assemble the components ----------------------------------------------
game.bin: game.asm ../../lib/crt0_entry.asm
	$(CPUASM) game.asm game.bin

# Placeholder PPU / APU asset blobs (replace with ppuasm/apuasm output).
gfx.bin:
	printf 'PPU-BLOB' > gfx.bin
	dd if=/dev/zero bs=1 count=248 >> gfx.bin 2>/dev/null   # pad to 256
snd.bin:
	printf 'APU-BLOB' > snd.bin
	dd if=/dev/zero bs=1 count=248 >> snd.bin 2>/dev/null

# --- dev side: link the flat segments into an ELF (no signing key) --------
elf: game.elf
game.elf: game.bin gfx.bin snd.bin
	$(ZPLINK) -o game.elf \
	    --cpu game.bin@0x8000 --ppu gfx.bin@0x100000 --apu snd.bin@0x200000 \
	    --entry 0x8000

# --- mastering: sign the ELF into a shippable .zpb ROM --------------------
# On real hardware this runs at HQ with the production key; here it uses the
# development key bundled in zpbuild, which the emulator accepts.
rom: game.zpb
game.zpb: game.elf
	$(ZPBUILD) game.elf -o game.zpb --title "Link Demo" --dev "ZP"

clean:
	rm -f game.bin gfx.bin snd.bin game.elf game.zpb

.PHONY: all elf rom clean
