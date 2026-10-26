// Unit tests for FlatMemory: byte access, 16-bit reads, and stack push/pop.
// The CPU only uses read() and write(); read16, push16, and pop16 are covered
// here and nowhere else.

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
    CHECK(view.read16(0x0042) == 0x0099);
}


// --- read16 ---

TEST_CASE("read16 is little-endian", "[memory]") {
    FlatMemory memory;
    memory.write(0x2000, 0x34);    // low byte
    memory.write(0x2001, 0x12);    // high byte
    CHECK(memory.read16(0x2000) == 0x1234);
}

TEST_CASE("read16 at 0xFFFF wraps to 0x0000 for the high byte", "[memory]") {
    FlatMemory memory;
    memory.write(0xFFFF, 0x34);    // low byte
    memory.write(0x0000, 0x12);    // high byte
    CHECK(memory.read16(0xFFFF) == 0x1234);
}


// --- push16 ---

TEST_CASE("push16 lowers SP by 2 and stores low byte at SP", "[memory][stack]") {
    FlatMemory memory;
    uint16_t sp = 0x2400;

    memory.push16(sp, 0x1234);
    CHECK(sp == 0x23FE);
    CHECK(memory.read(0x23FE) == 0x34);    // low byte
    CHECK(memory.read(0x23FF) == 0x12);    // high byte
}

TEST_CASE("push16 does not touch memory outside the two bytes", "[memory][stack]") {
    FlatMemory memory;
    uint16_t sp = 0x2400;

    memory.push16(sp, 0xFFFF);
    CHECK(memory.read(0x23FD) == 0x00);
    CHECK(memory.read(0x2400) == 0x00);
}

TEST_CASE("push16 wraps SP below 0x0000", "[memory][stack]") {
    SECTION("SP = 0x0001: value splits across 0xFFFF and 0x0000") {
        FlatMemory memory;
        uint16_t sp = 0x0001;
        memory.push16(sp, 0x1234);
        CHECK(sp == 0xFFFF);
        CHECK(memory.read(0xFFFF) == 0x34);    // low byte
        CHECK(memory.read(0x0000) == 0x12);    // high byte
    }

    SECTION("SP = 0x0000: value lands at the top of memory") {
        FlatMemory memory;
        uint16_t sp = 0x0000;
        memory.push16(sp, 0x1234);
        CHECK(sp == 0xFFFE);
        CHECK(memory.read(0xFFFE) == 0x34);
        CHECK(memory.read(0xFFFF) == 0x12);
    }
}


// --- pop16 ---

TEST_CASE("pop16 reads low byte at SP and raises SP by 2", "[memory][stack]") {
    FlatMemory memory;
    memory.write(0x23FE, 0x34);    // low byte
    memory.write(0x23FF, 0x12);    // high byte
    uint16_t sp = 0x23FE;

    CHECK(memory.pop16(sp) == 0x1234);
    CHECK(sp == 0x2400);
}

TEST_CASE("pop16 leaves the popped bytes in memory", "[memory][stack]") {
    FlatMemory memory;
    memory.write(0x23FE, 0x34);
    memory.write(0x23FF, 0x12);
    uint16_t sp = 0x23FE;

    memory.pop16(sp);
    CHECK(memory.read(0x23FE) == 0x34);
    CHECK(memory.read(0x23FF) == 0x12);
}

TEST_CASE("pop16 wraps SP above 0xFFFF", "[memory][stack]") {
    SECTION("SP = 0xFFFF: value splits across 0xFFFF and 0x0000") {
        FlatMemory memory;
        memory.write(0xFFFF, 0x34);    // low byte
        memory.write(0x0000, 0x12);    // high byte
        uint16_t sp = 0xFFFF;
        CHECK(memory.pop16(sp) == 0x1234);
        CHECK(sp == 0x0001);
    }

    SECTION("SP = 0xFFFE: SP wraps to 0x0000") {
        FlatMemory memory;
        memory.write(0xFFFE, 0x34);
        memory.write(0xFFFF, 0x12);
        uint16_t sp = 0xFFFE;
        CHECK(memory.pop16(sp) == 0x1234);
        CHECK(sp == 0x0000);
    }
}


// --- push16 + pop16 together ---

TEST_CASE("pop16 returns values in reverse push order", "[memory][stack]") {
    FlatMemory memory;
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
    FlatMemory memory;
    uint16_t sp = 0x0001;

    memory.push16(sp, 0xBEEF);
    CHECK(memory.pop16(sp) == 0xBEEF);
    CHECK(sp == 0x0001);
}
