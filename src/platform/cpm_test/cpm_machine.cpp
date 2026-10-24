#include <array>
#include <cstdio>

#include "cpm_machine.hpp"

// The test machine's memory: 64 KB of RAM
static std::array<uint8_t, 0x10000> ram{};

// START PLACEHOLDERS
// Memory (core/memory.hpp) - placeholder (each program defines its functions)

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
// END PLACEHOLDERS

// Copy program to PROGRAM_START and set up the CPU
bool CpmMachine::load(const std::vector<uint8_t>& program) {

    if (program.size() > ram.size() - PROGRAM_START) {
        return false; // program too big
    }

    // Copy program into memory
    for (size_t i = 0; i < program.size(); i++) {
        ram[PROGRAM_START + i] = program[i];
    }

    // Start at the program's first instruction
    cpu.set_pc(PROGRAM_START);

    // Set top of usable memory (low byte first)
    ram[MEMORY_TOP_PTR]     = MEMORY_TOP & 0xFF;
    ram[MEMORY_TOP_PTR + 1] = MEMORY_TOP >> 8;
    cpu.set_sp(MEMORY_TOP);

    return true;
}

// Run program until it jumps to EXIT_ADDR or CPU halts (returns false)
bool CpmMachine::run() {
    while (cpu.get_pc() != EXIT_ADDR) {

        if (cpu.get_pc() == BDOS_ADDR) { // program executed "CALL BDOS_ADDR"
            print_output();
            return_to_caller();
        } else if (cpu.is_halted()) {
            return false;
        } else {
            cycles += cpu.step(memory);
        }
    }
    return true;
}

// Print output character or string, depending on value stored in register C
void CpmMachine::print_output() {

    // Print character stored in register E
    if (cpu.get_c() == PRINT_CHAR) {
        std::putchar(cpu.get_e());

    // Print text starting at address in register DE, up to '$' character
    } else if (cpu.get_c() == PRINT_STRING) {
        uint16_t addr = (uint16_t)((cpu.get_d() << 8) | cpu.get_e());
        while (ram[addr] != '$') {
            std::putchar(ram[addr]);
            addr++;
        }
    }
    std::fflush(stdout); // so output isn't lost if the CPU crashes later
}

// Return to caller after the program's "CALL BDOS_ADDR" instruction (similar to RET)
void CpmMachine::return_to_caller() {
    uint16_t sp = cpu.get_sp();
    cpu.set_pc(memory.read16(sp));
    cpu.set_sp((uint16_t)(sp + 2));
}
