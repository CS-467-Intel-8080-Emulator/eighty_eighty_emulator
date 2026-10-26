#ifndef FLAT_MEMORY_HPP
#define FLAT_MEMORY_HPP

#include "memory_bus.hpp"

#include <cstdint>
#include <array>

class FlatMemory : public MemoryBus {
public:
    FlatMemory();
    // Read a single byte from 16‑bit address space
    uint8_t read(uint16_t addr) const override;

    // Write a single byte to 16‑bit address space
    void write(uint16_t addr, uint8_t value) override;

    // Read a 16‑bit little‑endian value (wraps at 0xFFFF)
    uint16_t read16(uint16_t addr) const override;

    // Push 16‑bit value onto stack
    void push16(uint16_t& sp, uint16_t value) override;

    // Pop 16‑bit value from stack
    uint16_t pop16(uint16_t& sp) override;

private:
    // Backing storage for the full 64 KB addressing space
    std::array<uint8_t, 65536> mem;
};


#endif
