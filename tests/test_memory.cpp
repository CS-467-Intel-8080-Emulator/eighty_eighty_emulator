// Unit tests for Memory: byte access, 16-bit reads, stack push/pop, and ROM loading.
// The CPU only uses read() and write(); read16, push16, pop16, and load_rom are
// covered here and nowhere else.

#include <array>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "../src/core/memory.hpp"


// --- Construction ---

TEST_CASE("New memory is all zeros", "[memory]") {
    Memory memory;

    int nonzero = 0;
    for (uint32_t addr = 0; addr <= 0xFFFF; addr++)
        if (memory.read((uint16_t)addr) != 0)
            nonzero++;
    CHECK(nonzero == 0);
}

TEST_CASE("Assigning a new Memory clears old contents", "[memory]") {
    // make_cpu() in the CPU tests relies on this to reset memory
    Memory memory;
    memory.write(0x0000, 0x11);
    memory.write(0x2400, 0x22);
    memory.write(0xFFFF, 0x33);

    memory = Memory();
    CHECK(memory.read(0x0000) == 0x00);
    CHECK(memory.read(0x2400) == 0x00);
    CHECK(memory.read(0xFFFF) == 0x00);
}


// --- read / write ---

TEST_CASE("write then read returns the byte", "[memory]") {
    Memory memory;
    memory.write(0x1234, 0xAB);
    CHECK(memory.read(0x1234) == 0xAB);
}

TEST_CASE("write only changes its own address", "[memory]") {
    Memory memory;
    memory.write(0x1234, 0xAB);
    CHECK(memory.read(0x1233) == 0x00);
    CHECK(memory.read(0x1235) == 0x00);
}

TEST_CASE("write overwrites the previous value", "[memory]") {
    Memory memory;
    memory.write(0x1234, 0xAB);
    memory.write(0x1234, 0x00);
    CHECK(memory.read(0x1234) == 0x00);
}

TEST_CASE("First and last addresses are usable", "[memory]") {
    Memory memory;
    memory.write(0x0000, 0x5A);
    memory.write(0xFFFF, 0xA5);
    CHECK(memory.read(0x0000) == 0x5A);
    CHECK(memory.read(0xFFFF) == 0xA5);
}

TEST_CASE("read works on const Memory", "[memory]") {
    Memory memory;
    memory.write(0x0042, 0x99);

    const Memory& view = memory;
    CHECK(view.read(0x0042) == 0x99);
    CHECK(view.read16(0x0042) == 0x0099);
}


// --- read16 ---

TEST_CASE("read16 is little-endian", "[memory]") {
    Memory memory;
    memory.write(0x2000, 0x34);    // low byte
    memory.write(0x2001, 0x12);    // high byte
    CHECK(memory.read16(0x2000) == 0x1234);
}

TEST_CASE("read16 at 0xFFFF wraps to 0x0000 for the high byte", "[memory]") {
    Memory memory;
    memory.write(0xFFFF, 0x34);    // low byte
    memory.write(0x0000, 0x12);    // high byte
    CHECK(memory.read16(0xFFFF) == 0x1234);
}


// --- push16 ---

TEST_CASE("push16 lowers SP by 2 and stores low byte at SP", "[memory][stack]") {
    Memory memory;
    uint16_t sp = 0x2400;

    memory.push16(sp, 0x1234);
    CHECK(sp == 0x23FE);
    CHECK(memory.read(0x23FE) == 0x34);    // low byte
    CHECK(memory.read(0x23FF) == 0x12);    // high byte
}

TEST_CASE("push16 does not touch memory outside the two bytes", "[memory][stack]") {
    Memory memory;
    uint16_t sp = 0x2400;

    memory.push16(sp, 0xFFFF);
    CHECK(memory.read(0x23FD) == 0x00);
    CHECK(memory.read(0x2400) == 0x00);
}

TEST_CASE("push16 wraps SP below 0x0000", "[memory][stack]") {
    SECTION("SP = 0x0001: value splits across 0xFFFF and 0x0000") {
        Memory memory;
        uint16_t sp = 0x0001;
        memory.push16(sp, 0x1234);
        CHECK(sp == 0xFFFF);
        CHECK(memory.read(0xFFFF) == 0x34);    // low byte
        CHECK(memory.read(0x0000) == 0x12);    // high byte
    }

    SECTION("SP = 0x0000: value lands at the top of memory") {
        Memory memory;
        uint16_t sp = 0x0000;
        memory.push16(sp, 0x1234);
        CHECK(sp == 0xFFFE);
        CHECK(memory.read(0xFFFE) == 0x34);
        CHECK(memory.read(0xFFFF) == 0x12);
    }
}


// --- pop16 ---

TEST_CASE("pop16 reads low byte at SP and raises SP by 2", "[memory][stack]") {
    Memory memory;
    memory.write(0x23FE, 0x34);    // low byte
    memory.write(0x23FF, 0x12);    // high byte
    uint16_t sp = 0x23FE;

    CHECK(memory.pop16(sp) == 0x1234);
    CHECK(sp == 0x2400);
}

TEST_CASE("pop16 leaves the popped bytes in memory", "[memory][stack]") {
    Memory memory;
    memory.write(0x23FE, 0x34);
    memory.write(0x23FF, 0x12);
    uint16_t sp = 0x23FE;

    memory.pop16(sp);
    CHECK(memory.read(0x23FE) == 0x34);
    CHECK(memory.read(0x23FF) == 0x12);
}

TEST_CASE("pop16 wraps SP above 0xFFFF", "[memory][stack]") {
    SECTION("SP = 0xFFFF: value splits across 0xFFFF and 0x0000") {
        Memory memory;
        memory.write(0xFFFF, 0x34);    // low byte
        memory.write(0x0000, 0x12);    // high byte
        uint16_t sp = 0xFFFF;
        CHECK(memory.pop16(sp) == 0x1234);
        CHECK(sp == 0x0001);
    }

    SECTION("SP = 0xFFFE: SP wraps to 0x0000") {
        Memory memory;
        memory.write(0xFFFE, 0x34);
        memory.write(0xFFFF, 0x12);
        uint16_t sp = 0xFFFE;
        CHECK(memory.pop16(sp) == 0x1234);
        CHECK(sp == 0x0000);
    }
}


// --- push16 + pop16 together ---

TEST_CASE("pop16 returns values in reverse push order", "[memory][stack]") {
    Memory memory;
    uint16_t sp = 0x2400;

    memory.push16(sp, 0x1111);
    memory.push16(sp, 0x2222);
    memory.push16(sp, 0x3333);

    CHECK(memory.pop16(sp) == 0x3333);
    CHECK(memory.pop16(sp) == 0x2222);
    CHECK(memory.pop16(sp) == 0x1111);
    CHECK(sp == 0x2400);
}

TEST_CASE("push16 and pop16 round-trip across the wrap point", "[memory][stack]") {
    Memory memory;
    uint16_t sp = 0x0001;

    memory.push16(sp, 0xBEEF);
    CHECK(memory.pop16(sp) == 0xBEEF);
    CHECK(sp == 0x0001);
}


// --- load_rom ---

TEST_CASE("load_rom copies bytes starting at start_addr", "[memory][rom]") {
    Memory memory;
    const std::array<uint8_t, 4> rom = {0xC3, 0x34, 0x12, 0x76};

    memory.load_rom(rom.data(), rom.size(), 0x0100);
    CHECK(memory.read(0x0100) == 0xC3);
    CHECK(memory.read(0x0101) == 0x34);
    CHECK(memory.read(0x0102) == 0x12);
    CHECK(memory.read(0x0103) == 0x76);
}

TEST_CASE("load_rom defaults to address 0x0000", "[memory][rom]") {
    Memory memory;
    const std::array<uint8_t, 2> rom = {0xAA, 0xBB};

    memory.load_rom(rom.data(), rom.size());
    CHECK(memory.read(0x0000) == 0xAA);
    CHECK(memory.read(0x0001) == 0xBB);
}

TEST_CASE("load_rom does not touch memory outside the ROM", "[memory][rom]") {
    Memory memory;
    memory.write(0x00FF, 0x11);
    memory.write(0x0104, 0x22);
    const std::array<uint8_t, 4> rom = {0x01, 0x02, 0x03, 0x04};

    memory.load_rom(rom.data(), rom.size(), 0x0100);
    CHECK(memory.read(0x00FF) == 0x11);
    CHECK(memory.read(0x0104) == 0x22);
}

TEST_CASE("load_rom with size 0 changes nothing", "[memory][rom]") {
    Memory memory;
    memory.write(0x0100, 0x11);
    const std::array<uint8_t, 1> rom = {0xFF};

    memory.load_rom(rom.data(), 0, 0x0100);
    CHECK(memory.read(0x0100) == 0x11);
}

TEST_CASE("load_rom wraps past 0xFFFF to 0x0000", "[memory][rom]") {
    Memory memory;
    const std::array<uint8_t, 3> rom = {0xAA, 0xBB, 0xCC};

    memory.load_rom(rom.data(), rom.size(), 0xFFFE);
    CHECK(memory.read(0xFFFE) == 0xAA);
    CHECK(memory.read(0xFFFF) == 0xBB);
    CHECK(memory.read(0x0000) == 0xCC);
}
