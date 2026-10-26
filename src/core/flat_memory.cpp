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

uint16_t FlatMemory::read16(uint16_t addr) const {
    // low byte first
    // Must wrap at 0xFFFF to avoid out‑of‑bounds
    uint8_t lo = mem[addr];
    uint8_t hi = mem[(addr + 1) & 0xFFFF];
    return (hi << 8) | lo;
}

void FlatMemory::push16(uint16_t& sp, uint16_t value) {
    // CALL pushes low byte first, then high byte
    // Stack grows downward and wraps at 0x0000
    sp = (sp - 2) & 0xFFFF;
    mem[sp] = value & 0xFF;                     // low byte
    mem[(sp + 1) & 0xFFFF] = (value >> 8) & 0xFF; // high byte
}

uint16_t FlatMemory::pop16(uint16_t& sp) {
    // RET pops low byte first, then high byte
    uint8_t lo = mem[sp];
    uint8_t hi = mem[(sp + 1) & 0xFFFF];
    sp = (sp + 2) & 0xFFFF;
    return (hi << 8) | lo;
}
