# zpcc - clang/LLVM-based C compiler for the DEF88186 CPU
#
# Requires an LLVM/clang development install (llvm-config on PATH).
# Tested with LLVM 22.

LLVM_CONFIG ?= llvm-config
CXX         ?= clang++

CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags) -Wall
LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags)
LIBS     := $(shell $(LLVM_CONFIG) --libs core irreader support) \
            $(shell $(LLVM_CONFIG) --system-libs)

TARGET := zpcc

all: $(TARGET)

$(TARGET): zpcc.cpp
	$(CXX) $(CXXFLAGS) zpcc.cpp $(LDFLAGS) $(LIBS) -o $@

clean:
	rm -f $(TARGET) *.ll *.o

.PHONY: all clean
