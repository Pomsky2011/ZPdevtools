// Test assembled PPU microcode binaries
// This program loads a .bin file and runs it on the PPU

#include "../ZeroPoint/include/ppu.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

using namespace ZeroPoint;

void printRegisters(const PPU& ppu, int start, int count) {
    for (int i = start; i < start + count && i < 64; i++) {
        std::cout << "R" << std::dec << i << "=0x"
                  << std::hex << std::setw(4) << std::setfill('0')
                  << ppu.getRegister(i) << " ";
        if ((i - start + 1) % 4 == 0) std::cout << "\n";
    }
    std::cout << std::dec << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program.bin> [max_cycles]" << std::endl;
        return 1;
    }

    const char* filename = argv[1];
    int max_cycles = (argc > 2) ? atoi(argv[2]) : 10000;

    // Load binary file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return 1;
    }

    std::vector<uint8_t> program((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
    file.close();

    std::cout << "Loaded " << program.size() << " bytes from " << filename << "\n\n";

    // Create PPU and load program
    PPU ppu;
    ppu.loadMicrocode(program.data(), program.size());
    ppu.setVBlankInterrupt(0x0000);
    ppu.setHBlankInterrupt(0x0000);
    ppu.start();

    // Run until halted or max cycles
    int cycles = 0;
    while (ppu.getState() != PPUState::Halted && cycles < max_cycles) {
        ppu.tick();
        cycles++;
    }

    // Print results
    std::cout << "Executed " << cycles << " cycles\n";
    std::cout << "Final state: ";

    switch (ppu.getState()) {
        case PPUState::Halted:
            std::cout << "HALTED\n";
            break;
        case PPUState::Running:
            std::cout << "RUNNING (hit max cycles)\n";
            break;
        default:
            std::cout << "OTHER\n";
            break;
    }

    std::cout << "\nRegisters:\n";
    printRegisters(ppu, 0, 8);

    std::cout << "\nSpecial Registers:\n";
    std::cout << "PC (R62) = 0x" << std::hex << ppu.getRegister(62) << std::dec << "\n";
    std::cout << "DP (R63) = 0x" << std::hex << ppu.getRegister(63) << std::dec << "\n";

    return 0;
}
