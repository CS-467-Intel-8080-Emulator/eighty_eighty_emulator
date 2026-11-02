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
};

// 16-bit helpers built on read()/write(). Each byte goes through its own
// read()/write() call, so any memory maps/addressing rules apply per-byte.

// Read a 16‑bit little‑endian value (low byte, then high byte)
inline uint16_t read16(const MemoryBus& memory, uint16_t addr) {
    uint8_t lo = memory.read(addr);
    uint8_t hi = memory.read(static_cast<uint16_t>(addr + 1));
    return static_cast<uint16_t>((hi << 8) | lo);
}

// Push 16‑bit value onto stack (high byte, then low byte)
// Stack grows downward and wraps at 0x0000
inline void push16(MemoryBus& memory, uint16_t& sp, uint16_t value) {
    sp = static_cast<uint16_t>(sp - 2);
    memory.write(static_cast<uint16_t>(sp + 1), (value >> 8) & 0xFF);
    memory.write(sp, value & 0xFF);
}

// Pop 16-bit value from stack (low byte, then high byte)
inline uint16_t pop16(const MemoryBus& memory, uint16_t& sp) {
    uint16_t value = read16(memory, sp);
    sp = static_cast<uint16_t>(sp + 2);
    return value;
}

#endif
