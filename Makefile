# ZeroPoint Development Tools Makefile

CC = gcc
CXX = g++
CFLAGS = -Wall -O2 -ansi -pedantic
CXXFLAGS = -Wall -O2 -std=c++17
LDFLAGS =

# Path to ZeroPoint source
ZEROPOINT_DIR = ../ZeroPoint
ZEROPOINT_INCLUDE = $(ZEROPOINT_DIR)/include
ZEROPOINT_LIB = $(ZEROPOINT_DIR)/build_qt/libzeropoint_core.a

all: zpasm

zpasm: zpasm.c
	$(CC) $(CFLAGS) -o zpasm zpasm.c $(LDFLAGS)

test_assembled: test_assembled.cpp $(ZEROPOINT_LIB)
	$(CXX) $(CXXFLAGS) -I$(ZEROPOINT_INCLUDE) -o test_assembled test_assembled.cpp $(ZEROPOINT_LIB)

clean:
	rm -f zpasm test_assembled *.o examples/*.bin

install: zpasm
	install -m 755 zpasm /usr/local/bin/

.PHONY: all clean install
