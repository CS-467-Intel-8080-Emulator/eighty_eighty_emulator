// Memory implementation for tests, backed by a 64K array

#include "test_memory.hpp"

std::array<uint8_t, 0x10000> test_ram{};

uint8_t Memory::read(uint16_t addr) {
    return test_ram[addr];
}

void Memory::write(uint16_t addr, uint8_t value) {
    test_ram[addr] = value;
}

uint16_t Memory::read16(uint16_t addr) {
    return test_ram[addr] | (test_ram[(uint16_t)(addr + 1)] << 8);
}

void Memory::push16(uint16_t& sp, uint16_t value) {
    test_ram[--sp] = value >> 8;
    test_ram[--sp] = value & 0xFF;
}
