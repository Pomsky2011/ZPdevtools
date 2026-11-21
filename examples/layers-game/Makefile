# layers-game moved: the game is the game template (../../game-template, the ZPgameTemplate repo), its art PNGs converted by scripts/png2zp.py.
# This Makefile builds it from here, so the old commands still work:
#
#   make              # build/game.zpb with zpcc          make CC=dos       # with def88186cc (use BUILD=build-dos to keep both)
#   make run          # zeropoint_sdl build/game.zpb      make clean
ZP     = ../..
TEMPLATE = $(ZP)/game-template
BUILD ?= $(abspath build)

all run clean elf:
	$(MAKE) -C $(TEMPLATE) ZPDEV=$(abspath $(ZP)) BUILD=$(BUILD) $(if $(CC),CC=$(CC),) $@

.PHONY: all run clean elf
