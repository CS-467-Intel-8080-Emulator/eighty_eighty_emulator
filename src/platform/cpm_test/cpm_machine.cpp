#include "cpm_machine.hpp"

#include <array>
#include <cstdio>

// The test machine's memory: 64 KB of RAM
static std::array<uint8_t, 0x10000> ram{};

// Memory (core/memory.hpp) is a placeholder: it only declares these functions, and each
// program that runs the CPU defines them with the memory behavior it needs (the unit tests
// do the same in tests/test_memory.cpp). src/core/memory.cpp is left out of emu so these
// don't collide with it. If Memory becomes an interface like IOBus, these definitions
// become a CpmMemory class that the machine passes to the CPU instead.

// Read one byte of memory
uint8_t Memory::read(uint16_t addr) {
    return ram[addr];
}

// Write one byte to memory
void Memory::write(uint16_t addr, uint8_t value) {
    ram[addr] = value;
}

// Read one word of memory, low-byte first (little endian)
uint16_t Memory::read16(uint16_t addr) {
    return ram[addr] | (ram[(uint16_t)(addr + 1)] << 8); // wrap 0xFFFF + 1 around to 0x0000
}

// Push downward onto stack, (high byte, then low byte)
void Memory::push16(uint16_t& sp, uint16_t value) {
    ram[--sp] = value >> 8;
    ram[--sp] = value & 0xFF;
}

bool CpmMachine::load(const std::vector<uint8_t>& program) {
    
    // Load program, starting at address 0x0100
    if (program.size() > ram.size() - 0x0100) {
        return false;
    }
    for (size_t i = 0; i < program.size(); i++) {
        ram[0x0100 + i] = program[i];
    }

    // CP/M stores the top of usable memory (0xF000) at 0x0006-0x0007
    ram[0x0006] = 0x00;
    ram[0x0007] = 0xF0;

    cpu.set_pc(0x0100);
    cpu.set_sp(0xF000);
    return true;
}

bool CpmMachine::run() {
    // Check pc before each instruction, so CP/M's addresses are caught before they run
    while (cpu.get_pc() != 0x0000) {    // 0x0000 means the program is done
        if (cpu.get_pc() == 0x0005) {
            bdos();
        } else if (cpu.is_halted()) {
            return false;
        } else {
            cycles_ += cpu.step(memory);
        }
    }
    return true;
}

void CpmMachine::bdos() {
    if (cpu.get_c() == 2) {
        // Print the character in E
        std::putchar(cpu.get_e());
    } else if (cpu.get_c() == 9) {
        // Print the text at address DE, up to a '$'
        uint16_t addr = (uint16_t)((cpu.get_d() << 8) | cpu.get_e());
        while (ram[addr] != '$') {
            std::putchar(ram[addr]);
            addr++;
        }
    }
    std::fflush(stdout);    // so output isn't lost if the CPU crashes later

    // Return to the program, the same way a RET instruction would
    uint16_t sp = cpu.get_sp();
    cpu.set_pc(memory.read16(sp));
    cpu.set_sp((uint16_t)(sp + 2));
}
