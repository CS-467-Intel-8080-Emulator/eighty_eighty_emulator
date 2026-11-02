// Unit tests for FlatMemory: construction and byte read/write.
// The 16-bit helpers from memory_bus.hpp are tested in test_memory_bus.cpp.

#include <array>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "../src/core/flat_memory.hpp"


// --- Construction ---

TEST_CASE("New memory is all zeros", "[memory]") {
    FlatMemory memory;

    int nonzero = 0;
    for (uint32_t addr = 0; addr <= 0xFFFF; addr++)
        if (memory.read((uint16_t)addr) != 0)
            nonzero++;
    CHECK(nonzero == 0);
}

TEST_CASE("Assigning a new FlatMemory clears old contents", "[memory]") {
    // make_cpu() in the CPU tests relies on this to reset memory
    FlatMemory memory;
    memory.write(0x0000, 0x11);
    memory.write(0x2400, 0x22);
    memory.write(0xFFFF, 0x33);

    memory = FlatMemory();
    CHECK(memory.read(0x0000) == 0x00);
    CHECK(memory.read(0x2400) == 0x00);
    CHECK(memory.read(0xFFFF) == 0x00);
}


// --- read / write ---

TEST_CASE("write then read returns the byte", "[memory]") {
    FlatMemory memory;
    memory.write(0x1234, 0xAB);
    CHECK(memory.read(0x1234) == 0xAB);
}

TEST_CASE("write only changes its own address", "[memory]") {
    FlatMemory memory;
    memory.write(0x1234, 0xAB);
    CHECK(memory.read(0x1233) == 0x00);
    CHECK(memory.read(0x1235) == 0x00);
}

TEST_CASE("write overwrites the previous value", "[memory]") {
    FlatMemory memory;
    memory.write(0x1234, 0xAB);
    memory.write(0x1234, 0x00);
    CHECK(memory.read(0x1234) == 0x00);
}

TEST_CASE("First and last addresses are usable", "[memory]") {
    FlatMemory memory;
    memory.write(0x0000, 0x5A);
    memory.write(0xFFFF, 0xA5);
    CHECK(memory.read(0x0000) == 0x5A);
    CHECK(memory.read(0xFFFF) == 0xA5);
}

TEST_CASE("read works on const FlatMemory", "[memory]") {
    FlatMemory memory;
    memory.write(0x0042, 0x99);

    const FlatMemory& view = memory;
    CHECK(view.read(0x0042) == 0x99);
}


// --- load_rom ---
// Deprecated tests. Original `Memory` implementation had a `load_rom` member
// function that was removed when `Memory` was replaced with `MemoryBus`.
// Keeping for future reference.

// TEST_CASE("load_rom copies bytes starting at start_addr", "[memory][rom]") {
//     Memory memory;
//     const std::array<uint8_t, 4> rom = {0xC3, 0x34, 0x12, 0x76};

//     memory.load_rom(rom.data(), rom.size(), 0x0100);
//     CHECK(memory.read(0x0100) == 0xC3);
//     CHECK(memory.read(0x0101) == 0x34);
//     CHECK(memory.read(0x0102) == 0x12);
//     CHECK(memory.read(0x0103) == 0x76);
// }

// TEST_CASE("load_rom defaults to address 0x0000", "[memory][rom]") {
//     Memory memory;
//     const std::array<uint8_t, 2> rom = {0xAA, 0xBB};

//     memory.load_rom(rom.data(), rom.size());
//     CHECK(memory.read(0x0000) == 0xAA);
//     CHECK(memory.read(0x0001) == 0xBB);
// }

// TEST_CASE("load_rom does not touch memory outside the ROM", "[memory][rom]") {
//     Memory memory;
//     memory.write(0x00FF, 0x11);
//     memory.write(0x0104, 0x22);
//     const std::array<uint8_t, 4> rom = {0x01, 0x02, 0x03, 0x04};

//     memory.load_rom(rom.data(), rom.size(), 0x0100);
//     CHECK(memory.read(0x00FF) == 0x11);
//     CHECK(memory.read(0x0104) == 0x22);
// }

// TEST_CASE("load_rom with size 0 changes nothing", "[memory][rom]") {
//     Memory memory;
//     memory.write(0x0100, 0x11);
//     const std::array<uint8_t, 1> rom = {0xFF};

//     memory.load_rom(rom.data(), 0, 0x0100);
//     CHECK(memory.read(0x0100) == 0x11);
// }

// TEST_CASE("load_rom wraps past 0xFFFF to 0x0000", "[memory][rom]") {
//     Memory memory;
//     const std::array<uint8_t, 3> rom = {0xAA, 0xBB, 0xCC};

//     memory.load_rom(rom.data(), rom.size(), 0xFFFE);
//     CHECK(memory.read(0xFFFE) == 0xAA);
//     CHECK(memory.read(0xFFFF) == 0xBB);
//     CHECK(memory.read(0x0000) == 0xCC);
// }
