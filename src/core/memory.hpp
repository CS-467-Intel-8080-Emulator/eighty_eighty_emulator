#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>
#include <array>

class Memory {
public:
    Memory();

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);

    uint16_t read16(uint16_t addr) const;

    void push16(uint16_t& sp, uint16_t value);
    uint16_t pop16(uint16_t& sp);

    void load_rom(const uint8_t* data, size_t size, uint16_t start_addr = 0);

private:
    std::array<uint8_t, 65536> mem;
};

#endif
