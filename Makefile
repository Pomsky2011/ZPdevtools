# ZeroPoint Development Tools Makefile
# Compatible with MS-DOS 4.01+ (use Makefile.dos for DOS builds)

CC = gcc
# Use C89/ANSI C for MS-DOS compatibility
CFLAGS = -Wall -O2 -std=c89 -pedantic -Isrc
LDFLAGS =

SRC = src

# Tools
ASSEMBLERS   = ppuasm apuasm cpuasm
ROM_TOOLS    = rombuilder rominspect
DISASSEMBLERS= cpudisasm ppudisasm apudisasm
CONVERTERS   = wav2mmp
UTILITIES    = hexview
ALL_TOOLS    = $(ASSEMBLERS) $(ROM_TOOLS) $(DISASSEMBLERS) $(CONVERTERS) $(UTILITIES)

all: $(ALL_TOOLS)

# Assemblers
ppuasm: $(SRC)/ppuasm.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

apuasm: $(SRC)/apuasm.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

cpuasm: $(SRC)/cpuasm.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# ROM Tools
rombuilder: $(SRC)/rombuilder.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

rominspect: $(SRC)/rominspect.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Disassemblers
cpudisasm: $(SRC)/cpudisasm.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

ppudisasm: $(SRC)/ppudisasm.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

apudisasm: $(SRC)/apudisasm.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Converters
wav2mmp: $(SRC)/wav2mmp.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lm

# Utilities
hexview: $(SRC)/hexview.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Convenience targets
assemblers: $(ASSEMBLERS)
	@echo "Assemblers built: $(ASSEMBLERS)"

disassemblers: $(DISASSEMBLERS)
	@echo "Disassemblers built: $(DISASSEMBLERS)"

rom-tools: $(ROM_TOOLS)
	@echo "ROM tools built: $(ROM_TOOLS)"

utilities: $(UTILITIES)
	@echo "Utilities built: $(UTILITIES)"

# Clean
clean:
	rm -f $(ALL_TOOLS) *.o src/*.o examples/*/*.bin

# Install
install: $(ALL_TOOLS)
	install -m 755 $(ALL_TOOLS) /usr/local/bin/

# Help
help:
	@echo "ZeroPoint Development Tools"
	@echo ""
	@echo "Targets:"
	@echo "  all            - Build all tools (default)"
	@echo "  assemblers     - Build assemblers only (ppuasm, apuasm, cpuasm)"
	@echo "  disassemblers  - Build disassemblers (cpudisasm, ppudisasm, apudisasm)"
	@echo "  rom-tools      - Build ROM tools (rombuilder, rominspect)"
	@echo "  utilities      - Build utilities (hexview, wav2mmp)"
	@echo "  clean          - Remove all built files"
	@echo "  install        - Install tools to /usr/local/bin"
	@echo ""
	@echo "Individual tools:"
	@echo "  Assemblers:    ppuasm, apuasm, cpuasm"
	@echo "  Disassemblers: cpudisasm, ppudisasm, apudisasm"
	@echo "  ROM Tools:     rombuilder, rominspect"
	@echo "  Converters:    wav2mmp"
	@echo "  Utilities:     hexview"

.PHONY: all assemblers disassemblers rom-tools utilities clean install help
