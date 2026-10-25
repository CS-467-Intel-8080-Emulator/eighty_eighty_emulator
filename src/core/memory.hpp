#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstddef>
#include <cstdint>
#include <array>

class Memory {
public:
    Memory();
    // Read a single byte from 16‑bit address space
    uint8_t read(uint16_t addr) const;

    // Write a single byte to 16‑bit address space
    void write(uint16_t addr, uint8_t value);

    // Read a 16‑bit little‑endian value (wraps at 0xFFFF)
    uint16_t read16(uint16_t addr) const;

    // Push 16‑bit value onto stack
    void push16(uint16_t& sp, uint16_t value);

    // Pop 16‑bit value from stack
    uint16_t pop16(uint16_t& sp);

    // Load ROM data into memory with wraparound
    void load_rom(const uint8_t* data, std::size_t size, uint16_t start_addr = 0);

private:
    // Full 64 KB memory space
    std::array<uint8_t, 65536> mem;
};


#endif
