#include "memory.hpp"

Memory::Memory() {
    // Initialize RAM to zero
    mem.fill(0);
}

uint8_t Memory::read(uint16_t addr) const {
    // Direct 16‑bit indexed read
    return mem[addr];
}

void Memory::write(uint16_t addr, uint8_t value) {
    // Direct 16‑bit indexed write
    mem[addr] = value;
}

uint16_t Memory::read16(uint16_t addr) const {
    // low byte first
    // Must wrap at 0xFFFF to avoid out‑of‑bounds
    uint8_t lo = mem[addr];
    uint8_t hi = mem[(addr + 1) & 0xFFFF];
    return (hi << 8) | lo;
}

void Memory::push16(uint16_t& sp, uint16_t value) {
    // CALL pushes low byte first, then high byte
    // Stack grows downward and wraps at 0x0000
    sp = (sp - 2) & 0xFFFF;
    mem[sp] = value & 0xFF;                     // low byte
    mem[(sp + 1) & 0xFFFF] = (value >> 8) & 0xFF; // high byte
}

uint16_t Memory::pop16(uint16_t& sp) {
    // RET pops low byte first, then high byte
    uint8_t lo = mem[sp];
    uint8_t hi = mem[(sp + 1) & 0xFFFF];
    sp = (sp + 2) & 0xFFFF;
    return (hi << 8) | lo;
}

void Memory::load_rom(const uint8_t* data, std::size_t size, uint16_t start_addr) {
    // Load ROM into memory with wraparound at 0xFFFF
    for (std::size_t i = 0; i < size; i++)
        mem[(start_addr + i) & 0xFFFF] = data[i];
}
