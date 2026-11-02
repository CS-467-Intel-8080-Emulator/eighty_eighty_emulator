// Unit tests for the 16-bit helpers in memory_bus.hpp: read16, push16, pop16.
// FlatMemory is used as the underlying memory instance.

#include <cstdint>
#include <catch2/catch_test_macros.hpp>
#include "../src/core/flat_memory.hpp"


// --- read16 ---

TEST_CASE("read16 works on const MemoryBus", "[memory]") {
    FlatMemory memory;
    memory.write(0x0042, 0x99);

    const MemoryBus& view = memory;
    CHECK(read16(view, 0x0042) == 0x0099);
}

TEST_CASE("read16 is little-endian", "[memory]") {
    FlatMemory memory;
    memory.write(0x2000, 0x34);    // low byte
    memory.write(0x2001, 0x12);    // high byte
    CHECK(read16(memory, 0x2000) == 0x1234);
}

TEST_CASE("read16 at 0xFFFF wraps to 0x0000 for the high byte", "[memory]") {
    FlatMemory memory;
    memory.write(0xFFFF, 0x34);    // low byte
    memory.write(0x0000, 0x12);    // high byte
    CHECK(read16(memory, 0xFFFF) == 0x1234);
}


// --- push16 ---

TEST_CASE("push16 lowers SP by 2 and stores low byte at SP", "[memory][stack]") {
    FlatMemory memory;
    uint16_t sp = 0x2400;

    push16(memory, sp, 0x1234);
    CHECK(sp == 0x23FE);
    CHECK(memory.read(0x23FE) == 0x34);    // low byte
    CHECK(memory.read(0x23FF) == 0x12);    // high byte
}

TEST_CASE("push16 does not touch memory outside the two bytes", "[memory][stack]") {
    FlatMemory memory;
    uint16_t sp = 0x2400;

    push16(memory, sp, 0xFFFF);
    CHECK(memory.read(0x23FD) == 0x00);
    CHECK(memory.read(0x2400) == 0x00);
}

TEST_CASE("push16 wraps SP below 0x0000", "[memory][stack]") {
    SECTION("SP = 0x0001: value splits across 0xFFFF and 0x0000") {
        FlatMemory memory;
        uint16_t sp = 0x0001;
        push16(memory, sp, 0x1234);
        CHECK(sp == 0xFFFF);
        CHECK(memory.read(0xFFFF) == 0x34);    // low byte
        CHECK(memory.read(0x0000) == 0x12);    // high byte
    }

    SECTION("SP = 0x0000: value lands at the top of memory") {
        FlatMemory memory;
        uint16_t sp = 0x0000;
        push16(memory, sp, 0x1234);
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

    CHECK(pop16(memory, sp) == 0x1234);
    CHECK(sp == 0x2400);
}

TEST_CASE("pop16 leaves the popped bytes in memory", "[memory][stack]") {
    FlatMemory memory;
    memory.write(0x23FE, 0x34);
    memory.write(0x23FF, 0x12);
    uint16_t sp = 0x23FE;

    pop16(memory, sp);
    CHECK(memory.read(0x23FE) == 0x34);
    CHECK(memory.read(0x23FF) == 0x12);
}

TEST_CASE("pop16 wraps SP above 0xFFFF", "[memory][stack]") {
    SECTION("SP = 0xFFFF: value splits across 0xFFFF and 0x0000") {
        FlatMemory memory;
        memory.write(0xFFFF, 0x34);    // low byte
        memory.write(0x0000, 0x12);    // high byte
        uint16_t sp = 0xFFFF;
        CHECK(pop16(memory, sp) == 0x1234);
        CHECK(sp == 0x0001);
    }

    SECTION("SP = 0xFFFE: SP wraps to 0x0000") {
        FlatMemory memory;
        memory.write(0xFFFE, 0x34);
        memory.write(0xFFFF, 0x12);
        uint16_t sp = 0xFFFE;
        CHECK(pop16(memory, sp) == 0x1234);
        CHECK(sp == 0x0000);
    }
}


// --- push16 + pop16 together ---

TEST_CASE("pop16 returns values in reverse push order", "[memory][stack]") {
    FlatMemory memory;
    uint16_t sp = 0x2400;

    push16(memory, sp, 0x1111);
    push16(memory, sp, 0x2222);
    push16(memory, sp, 0x3333);

    CHECK(pop16(memory, sp) == 0x3333);
    CHECK(pop16(memory, sp) == 0x2222);
    CHECK(pop16(memory, sp) == 0x1111);
    CHECK(sp == 0x2400);
}

TEST_CASE("push16 and pop16 round-trip across the wrap point", "[memory][stack]") {
    FlatMemory memory;
    uint16_t sp = 0x0001;

    push16(memory, sp, 0xBEEF);
    CHECK(pop16(memory, sp) == 0xBEEF);
    CHECK(sp == 0x0001);
}
