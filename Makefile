# ZeroPoint Development Tools Makefile
# Compatible with MS-DOS 4.01+ (use Makefile.dos for DOS builds)

CC = gcc
CXX = g++
# Use C89/ANSI C for MS-DOS compatibility
CFLAGS = -Wall -O2 -std=c89 -pedantic -I.
CXXFLAGS = -Wall -O2 -std=c++17
LDFLAGS =

# Path to ZeroPoint source
ZEROPOINT_DIR = ../ZeroPoint
ZEROPOINT_INCLUDE = $(ZEROPOINT_DIR)/include
ZEROPOINT_LIB = $(ZEROPOINT_DIR)/build_qt/libzeropoint_core.a

# Tools
ASSEMBLERS = ppuasm apuasm cpuasm
ROM_TOOLS = rombuilder rominspect
DISASSEMBLERS = cpudisasm ppudisasm apudisasm
CONVERTERS = wav2mmp
UTILITIES = hexview
ALL_TOOLS = $(ASSEMBLERS) $(ROM_TOOLS) $(DISASSEMBLERS) $(CONVERTERS) $(UTILITIES)

all: $(ALL_TOOLS)

# Assemblers
ppuasm: ppuasm.c
	$(CC) $(CFLAGS) -o ppuasm ppuasm.c $(LDFLAGS)

apuasm: apuasm.c
	$(CC) $(CFLAGS) -o apuasm apuasm.c $(LDFLAGS)

cpuasm: cpuasm.c
	$(CC) $(CFLAGS) -o cpuasm cpuasm.c $(LDFLAGS)

# ROM Tools
rombuilder: rombuilder.c
	$(CC) $(CFLAGS) -o rombuilder rombuilder.c $(LDFLAGS)

rominspect: rominspect.c
	$(CC) $(CFLAGS) -o rominspect rominspect.c $(LDFLAGS)

# Disassemblers
cpudisasm: cpudisasm.c
	$(CC) $(CFLAGS) -o cpudisasm cpudisasm.c $(LDFLAGS)

ppudisasm: ppudisasm.c
	$(CC) $(CFLAGS) -o ppudisasm ppudisasm.c $(LDFLAGS)

apudisasm: apudisasm.c
	$(CC) $(CFLAGS) -o apudisasm apudisasm.c $(LDFLAGS)

# Converters
wav2mmp: wav2mmp.c
	$(CC) $(CFLAGS) -o wav2mmp wav2mmp.c $(LDFLAGS) -lm

# Utilities
hexview: hexview.c
	$(CC) $(CFLAGS) -o hexview hexview.c $(LDFLAGS)

# Test program
test_assembled: test_assembled.cpp $(ZEROPOINT_LIB)
	$(CXX) $(CXXFLAGS) -I$(ZEROPOINT_INCLUDE) -o test_assembled test_assembled.cpp $(ZEROPOINT_LIB)

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
	rm -f $(ALL_TOOLS) test_assembled *.o examples/*/*.bin examples/*.bin

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
	@echo "  Assemblers:     ppuasm, apuasm, cpuasm"
	@echo "  Disassemblers:  cpudisasm, ppudisasm, apudisasm"
	@echo "  ROM Tools:      rombuilder, rominspect"
	@echo "  Converters:     wav2mmp"
	@echo "  Utilities:      hexview"

.PHONY: all assemblers disassemblers rom-tools utilities clean install help
