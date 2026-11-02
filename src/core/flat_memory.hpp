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

private:
    // Backing storage for the full 64 KB addressing space
    std::array<uint8_t, 65536> mem;
};


#endif
