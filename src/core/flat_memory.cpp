#include "flat_memory.hpp"

FlatMemory::FlatMemory() {
    // Initialize RAM to zero
    mem.fill(0);
}

uint8_t FlatMemory::read(uint16_t addr) const {
    // Direct 16‑bit indexed read
    return mem[addr];
}

void FlatMemory::write(uint16_t addr, uint8_t value) {
    // Direct 16‑bit indexed write
    mem[addr] = value;
}
