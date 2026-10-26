#ifndef MEMORY_BUS_HPP
#define MEMORY_BUS_HPP

#include <cstdint>

// Memory read/write interface. The machine (e.g. Space Invaders) implements
// this with memory map (addressing rules).
class MemoryBus {
public:
    virtual ~MemoryBus() = default;

    // Read a single byte from 16‑bit address space
    virtual uint8_t read(uint16_t addr) const = 0;

    // Write a single byte to 16‑bit address space
    virtual void write(uint16_t addr, uint8_t value) = 0;

    // Read a 16‑bit little‑endian value (wraps at 0xFFFF)
    virtual uint16_t read16(uint16_t addr) const = 0;

    // Push 16‑bit value onto stack
    virtual void push16(uint16_t& sp, uint16_t value) = 0;

    // Pop 16‑bit value from stack
    virtual uint16_t pop16(uint16_t& sp) = 0;
};

#endif
