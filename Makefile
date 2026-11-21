# ZeroPoint Development Tools Makefile
# Compatible with MS-DOS 4.01+ (use Makefile.dos for DOS builds)

CC = gcc
# Use C89/ANSI C for MS-DOS compatibility
CFLAGS   = -Wall -O2 -std=c89 -pedantic -Isrc
LDFLAGS  =

SRC    = src
BINDIR = executables

# Tools
ASSEMBLERS    = ppuasm apuasm cpuasm
ROM_TOOLS     = rominspect
LINKERS       = zplink
SIGNERS       = zpbuild
DISASSEMBLERS = cpudisasm ppudisasm apudisasm
UTILITIES     = hexview
ALL_TOOLS     = $(ASSEMBLERS) $(ROM_TOOLS) $(LINKERS) $(SIGNERS) $(DISASSEMBLERS) $(UTILITIES)
ALL_BINS      = $(addprefix $(BINDIR)/,$(ALL_TOOLS))

all: $(BINDIR) $(ALL_BINS)

$(BINDIR):
	mkdir -p $@

# Assemblers
$(BINDIR)/ppuasm: $(SRC)/ppuasm.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/apuasm: $(SRC)/apuasm.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/cpuasm: $(SRC)/cpuasm.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# ROM Tools
$(BINDIR)/rominspect: $(SRC)/rominspect.c $(SRC)/zpsha256.h | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Linker (depends on the SHA-256 / RSA / key headers in src/)
# zplink: developer-side linker, no signing key -> only needs the ELF writer.
$(BINDIR)/zplink: $(SRC)/zplink.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SRC)/zplink.c $(LDFLAGS)

# zpbuild: HQ mastering/signing step -> pulls in the SHA-256 + RSA key material.
$(BINDIR)/zpbuild: $(SRC)/zpbuild.c $(SRC)/zpsha256.h $(SRC)/zpblake2s.h $(SRC)/zprsa.h $(SRC)/zpkey.h | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SRC)/zpbuild.c $(LDFLAGS)

# Disassemblers
$(BINDIR)/cpudisasm: $(SRC)/cpudisasm.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/ppudisasm: $(SRC)/ppudisasm.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/apudisasm: $(SRC)/apudisasm.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Utilities
$(BINDIR)/hexview: $(SRC)/hexview.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Convenience targets
assemblers: $(addprefix $(BINDIR)/,$(ASSEMBLERS))
	@echo "Assemblers built: $(ASSEMBLERS)"

disassemblers: $(addprefix $(BINDIR)/,$(DISASSEMBLERS))
	@echo "Disassemblers built: $(DISASSEMBLERS)"

rom-tools: $(addprefix $(BINDIR)/,$(ROM_TOOLS))
	@echo "ROM tools built: $(ROM_TOOLS)"

linkers: $(addprefix $(BINDIR)/,$(LINKERS))
	@echo "Linkers built: $(LINKERS)"

utilities: $(addprefix $(BINDIR)/,$(UTILITIES))
	@echo "Utilities built: $(UTILITIES)"

# Assemble the PPU demos (examples/ppu/*.asm -> *.bin; nothing built is tracked)
examples: $(BINDIR)/ppuasm
	for f in examples/ppu/*.asm; do $(BINDIR)/ppuasm $$f -o $${f%.asm}.bin || exit 1; done

# Clean
clean:
	rm -f $(ALL_BINS) src/*.o examples/*/*.bin

# Install
install: $(ALL_BINS)
	install -m 755 $(ALL_BINS) /usr/local/bin/

# Help
help:
	@echo "ZeroPoint Development Tools"
	@echo ""
	@echo "Targets:"
	@echo "  all            - Build all tools (default)"
	@echo "  assemblers     - Build assemblers only (ppuasm, apuasm, cpuasm)"
	@echo "  disassemblers  - Build disassemblers (cpudisasm, ppudisasm, apudisasm)"
	@echo "  rom-tools      - Build ROM tools (rominspect)"
	@echo "  utilities      - Build utilities (hexview)"
	@echo "  examples       - Assemble the PPU demos in examples/ppu to .bin files"
	@echo "  check          - Build, then run the assembler, linker, signer and disassembler tests"
	@echo "  clean          - Remove all built files"
	@echo "  install        - Install tools to /usr/local/bin"
	@echo ""
	@echo "Binaries output to: $(BINDIR)/"
	@echo ""
	@echo "Individual tools:"
	@echo "  Assemblers:    ppuasm, apuasm, cpuasm"
	@echo "  Disassemblers: cpudisasm, ppudisasm, apudisasm"
	@echo "  ROM Tools:     rominspect, zplink, zpbuild"
	@echo "  Utilities:     hexview"

# The fast tool tests (the fuzzer, tests/asm/fuzz_asm.py, and the compiler suite, tests/cc/run.sh, are run separately).
check: all
	sh tests/asm/check.sh
	sh tests/asm/robust.sh
	python3 scripts/sync_cpuasm_cycles.py --check
	sh tests/c89/check.sh
	python3 tests/link/check.py
	python3 tests/disasm/check.py

.PHONY: all assemblers disassemblers rom-tools utilities examples clean install help check
