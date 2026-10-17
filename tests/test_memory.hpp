#ifndef TEST_MEMORY_HPP
#define TEST_MEMORY_HPP

#include <array>
#include <cstdint>

#include "../src/core/memory.hpp"

// Backing store for test Memory
extern std::array<uint8_t, 0x10000> test_ram;

#endif
