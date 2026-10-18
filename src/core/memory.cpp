#include "memory.hpp"

Memory::Memory() {
    mem.fill(0);
}

uint8_t Memory::read(uint16_t addr) const {
    return mem[addr];
}

void Memory::write(uint16_t addr, uint8_t value) {
    mem[addr] = value;
}

uint16_t Memory::read16(uint16_t addr) const {
    uint8_t lo = mem[addr];
    uint8_t hi = mem[addr + 1];
    return (hi << 8) | lo;
}

void Memory::push16(uint16_t& sp, uint16_t value) {
    // 8080 pushes high byte first
    mem[--sp] = (value >> 8) & 0xFF;
    mem[--sp] = value & 0xFF;
}

uint16_t Memory::pop16(uint16_t& sp) {
    uint8_t lo = mem[sp++];
    uint8_t hi = mem[sp++];
    return (hi << 8) | lo;
}

void Memory::load_rom(const uint8_t* data, size_t size, uint16_t start_addr) {
    for (size_t i = 0; i < size; i++)
        mem[start_addr + i] = data[i];
}
