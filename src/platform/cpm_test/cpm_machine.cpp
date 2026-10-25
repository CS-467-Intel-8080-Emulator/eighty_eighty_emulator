#include <cstdio>

#include "cpm_machine.hpp"

// Copy program to PROGRAM_START and set up the CPU
bool CpmMachine::load(const std::vector<uint8_t>& program) {

    if (program.size() > MEMORY_SIZE - PROGRAM_START) {
        return false; // program too big
    }

    // Copy program into memory
    memory.load_rom(program.data(), program.size(), PROGRAM_START);

    // Start at the program's first instruction
    cpu.set_pc(PROGRAM_START);

    // Set top of usable memory (low byte first)
    memory.write(MEMORY_TOP_PTR,     MEMORY_TOP & 0xFF);
    memory.write(MEMORY_TOP_PTR + 1, MEMORY_TOP >> 8);
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
        while (memory.read(addr) != '$') {
            std::putchar(memory.read(addr));
            addr++;
        }
    }
    std::fflush(stdout); // so output isn't lost if the CPU crashes later
}

// Return to caller after the program's "CALL BDOS_ADDR" instruction (similar to RET)
void CpmMachine::return_to_caller() {
    uint16_t sp = cpu.get_sp();
    cpu.set_pc(memory.pop16(sp));   // pop16 moves sp up past the return address
    cpu.set_sp(sp);
}
