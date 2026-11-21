# layers-game: builds the example ROM with the C layers library.
#
#   make              # build/game.zpb with zpcc (the clang compiler)
#   make CC=dos       # the same with def88186cc (-DZP_CC_DOS)
#   make run          # zeropoint_sdl build/game.zpb (SDL frontend from ../../../ZeroPoint/build/bin)
#   make clean
#
# Needs: ZPdevtools built (executables/, llvm_compiler/zpcc and/or c_compiler/def88186cc: `make` in each), python3.
ZP      = ../..
BIN     = $(ZP)/executables
CC      ?= zpcc
EMU     ?= $(ZP)/../ZeroPoint/build/bin/zeropoint_sdl
BUILD   = build

ifeq ($(CC),dos)
  COMPILE = $(ZP)/c_compiler/def88186cc -DZP_CC_DOS
else
  COMPILE = $(ZP)/llvm_compiler/zpcc
endif

all: $(BUILD)/game.zpb

# the microcode, padded to whole 256-byte pages (the Boot ROM loader copies pages); LAYERS_PAGES says how many
$(BUILD)/layers.bin: $(ZP)/ucode/layers/gen_layers.py
	mkdir -p $(BUILD)
	python3 $< -o $(BUILD)/layers.raw > /dev/null
	python3 -c "d=open('$(BUILD)/layers.raw','rb').read(); d+=bytes(-len(d)%256); open('$@','wb').write(d); open('$(BUILD)/pages.txt','w').write(str(len(d)//256))"

$(BUILD)/assets.bin $(BUILD)/assets.h: gen_assets.py
	python3 gen_assets.py $(BUILD)

$(BUILD)/main.asm: main.c $(BUILD)/assets.h $(BUILD)/layers.bin $(wildcard $(ZP)/include/zeropoint/*.h)
	$(COMPILE) -DLAYERS_PAGES=$$(cat $(BUILD)/pages.txt) -I $(ZP)/include -I $(BUILD) main.c -o $@

# crt0 (the entry stub), the compiled program, then the runtime it calls (peek/poke, COP, 32-bit arithmetic)
$(BUILD)/game.asm: $(BUILD)/main.asm
	printf '.org $$8000\n.include "$(abspath $(ZP))/lib/crt0_entry.asm"\n.include "$(abspath $<)"\n.include "$(abspath $(ZP))/lib/zp_mmio.asm"\n.include "$(abspath $(ZP))/lib/zp_cop.asm"\n.include "$(abspath $(ZP))/lib/zp_long.asm"\n' > $@

$(BUILD)/game.bin: $(BUILD)/game.asm
	$(BIN)/cpuasm $< $@

$(BUILD)/game.elf: $(BUILD)/game.bin $(BUILD)/layers.bin $(BUILD)/assets.bin
	$(BIN)/zplink -o $@ --cpu $(BUILD)/game.bin@0x8000 --ppu $(BUILD)/layers.bin@0x100000 --data $(BUILD)/assets.bin@0x110000 --entry 0x8000

# Signed with --codesize N --trailer 2: the console's BLAKE2s engine hashes the whole ROM at boot in a fraction of a second. Without them zpbuild uses the
# original single-region SHA-256 path, which the real Boot ROM computes in software: for a ROM this size it never finishes (the lightning splash stays up
# for good), though the native boot, which does not hash, runs it. N has to cover the CPU program (it starts at $8000).
$(BUILD)/game.zpb: $(BUILD)/game.elf
	$(BIN)/zpbuild $< -o $@ --title "LAYERS GAME" --dev ZP --codesize $$(python3 -c "import os; print((0x8000 + os.path.getsize('$(BUILD)/game.bin') + 255) // 256 * 256)") --trailer 2

run: $(BUILD)/game.zpb
	$(EMU) $<

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
