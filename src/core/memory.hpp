// THIS IS A PLACEHOLDER FILE TO REMOVE ERRORS IN CPU MEMORY CALLS

#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>

class Memory {
public:
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);
    uint16_t read16(uint16_t addr);
    void push16(uint16_t& sp, uint16_t value);
};

#endif
