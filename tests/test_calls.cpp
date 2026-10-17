// Unit tests for CALL/Ccc, RET/Rcc, RST, and PCHL instructions.

#include <cstdint>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include "cpu_test_helpers.hpp"

static Memory mem;

// Stack starts here, as in Space Invaders (top of work RAM)
static constexpr uint16_t STACK_TOP = 0x2400;

// Word stored little-endian at addr (low byte at addr, high byte at addr + 1)
static uint16_t stack_word(uint16_t addr) {
    return test_ram[addr] | (test_ram[(uint16_t)(addr + 1)] << 8);
}

// Places a word on the stack the way a push would leave it
static void put_stack_word(uint16_t addr, uint16_t value) {
    test_ram[addr] = value & 0xFF;
    test_ram[(uint16_t)(addr + 1)] = value >> 8;
}


// --- CALL ---

TEST_CASE("CALL pushes the return address and jumps", "[cpu][call]") {
    CPU cpu = make_cpu({0xCD, 0x34, 0x12});    // CALL 0x1234
    T::sp(cpu) = STACK_TOP;

    CHECK(cpu.step(mem) == 17);
    CHECK(T::pc(cpu) == 0x1234);
    CHECK(T::sp(cpu) == STACK_TOP - 2);
    CHECK(test_ram[STACK_TOP - 1] == 0x01);    // high byte of ORIGIN + 3
    CHECK(test_ram[STACK_TOP - 2] == 0x03);    // low byte of ORIGIN + 3
}

TEST_CASE("CALL does not change flags", "[cpu][call]") {
    CPU cpu = make_cpu({0xCD, 0x34, 0x12}, ALL_FLAGS);
    T::sp(cpu) = STACK_TOP;
    cpu.step(mem);
    CHECK(T::flags(cpu) == ALL_FLAGS);
}

TEST_CASE("CALL wraps SP below 0x0000", "[cpu][call]") {
    SECTION("SP = 0x0001: return address splits across 0x0000 and 0xFFFF") {
        CPU cpu = make_cpu({0xCD, 0x34, 0x12});
        T::sp(cpu) = 0x0001;
        cpu.step(mem);
        CHECK(T::sp(cpu) == 0xFFFF);
        CHECK(test_ram[0x0000] == 0x01);
        CHECK(test_ram[0xFFFF] == 0x03);
    }

    SECTION("SP = 0x0000: return address at the top of memory") {
        CPU cpu = make_cpu({0xCD, 0x34, 0x12});
        T::sp(cpu) = 0x0000;
        cpu.step(mem);
        CHECK(T::sp(cpu) == 0xFFFE);
        CHECK(stack_word(0xFFFE) == ORIGIN + 3);
    }
}


// --- Ccc ---

TEST_CASE("Ccc calls only when its condition holds", "[cpu][call]") {
    const Condition& cond = GENERATE(from_range(CONDITIONS));
    uint8_t opcode = 0xC4 | (cond.cc << 3);
    CAPTURE(cond.name);

    SECTION("condition true: pushes the return address and jumps") {
        CPU cpu = make_cpu({opcode, 0xCD, 0xAB}, cond.true_flags());
        T::sp(cpu) = STACK_TOP;
        CHECK(cpu.step(mem) == 17);
        CHECK(T::pc(cpu) == 0xABCD);
        CHECK(T::sp(cpu) == STACK_TOP - 2);
        CHECK(stack_word(STACK_TOP - 2) == ORIGIN + 3);
        CHECK(T::flags(cpu) == cond.true_flags());
    }

    SECTION("condition false: skips the operand and leaves the stack alone") {
        CPU cpu = make_cpu({opcode, 0xCD, 0xAB}, cond.false_flags());
        T::sp(cpu) = STACK_TOP;
        CHECK(cpu.step(mem) == 11);
        CHECK(T::pc(cpu) == ORIGIN + 3);
        CHECK(T::sp(cpu) == STACK_TOP);
        CHECK(stack_word(STACK_TOP - 2) == 0x0000);
        CHECK(T::flags(cpu) == cond.false_flags());
    }
}


// --- RET ---

TEST_CASE("RET pops PC from the stack", "[cpu][ret]") {
    CPU cpu = make_cpu({0xC9});                // RET
    T::sp(cpu) = STACK_TOP - 2;
    put_stack_word(STACK_TOP - 2, 0x1234);

    CHECK(cpu.step(mem) == 10);
    CHECK(T::pc(cpu) == 0x1234);
    CHECK(T::sp(cpu) == STACK_TOP);
}

TEST_CASE("RET wraps SP above 0xFFFF", "[cpu][ret]") {
    CPU cpu = make_cpu({0xC9});
    T::sp(cpu) = 0xFFFF;
    test_ram[0xFFFF] = 0x34;                   // low byte
    test_ram[0x0000] = 0x12;                   // high byte

    cpu.step(mem);
    CHECK(T::pc(cpu) == 0x1234);
    CHECK(T::sp(cpu) == 0x0001);
}

TEST_CASE("CALL then RET returns to the next instruction", "[cpu][call][ret]") {
    CPU cpu = make_cpu({0xCD, 0x00, 0x02}, ALL_FLAGS);    // CALL 0x0200
    test_ram[0x0200] = 0xC9;                               // RET
    T::sp(cpu) = STACK_TOP;

    cpu.step(mem);
    CHECK(T::pc(cpu) == 0x0200);
    cpu.step(mem);
    CHECK(T::pc(cpu) == ORIGIN + 3);
    CHECK(T::sp(cpu) == STACK_TOP);
    CHECK(T::flags(cpu) == ALL_FLAGS);
}


// --- Rcc ---

TEST_CASE("Rcc returns only when its condition holds", "[cpu][ret]") {
    const Condition& cond = GENERATE(from_range(CONDITIONS));
    uint8_t opcode = 0xC0 | (cond.cc << 3);
    CAPTURE(cond.name);

    SECTION("condition true: pops PC from the stack") {
        CPU cpu = make_cpu({opcode}, cond.true_flags());
        T::sp(cpu) = STACK_TOP - 2;
        put_stack_word(STACK_TOP - 2, 0x1234);
        CHECK(cpu.step(mem) == 11);
        CHECK(T::pc(cpu) == 0x1234);
        CHECK(T::sp(cpu) == STACK_TOP);
        CHECK(T::flags(cpu) == cond.true_flags());
    }

    SECTION("condition false: falls through and leaves SP alone") {
        CPU cpu = make_cpu({opcode}, cond.false_flags());
        T::sp(cpu) = STACK_TOP - 2;
        put_stack_word(STACK_TOP - 2, 0x1234);
        CHECK(cpu.step(mem) == 5);
        CHECK(T::pc(cpu) == ORIGIN + 1);
        CHECK(T::sp(cpu) == STACK_TOP - 2);
        CHECK(T::flags(cpu) == cond.false_flags());
    }
}


// --- RST ---

TEST_CASE("RST n pushes the return address and jumps to n*8", "[cpu][rst]") {
    uint8_t n = GENERATE(range(0, 8));
    uint8_t opcode = 0xC7 | (n << 3);
    CAPTURE(n);

    CPU cpu = make_cpu({opcode}, ALL_FLAGS);
    T::sp(cpu) = STACK_TOP;

    CHECK(cpu.step(mem) == 11);
    CHECK(T::pc(cpu) == n * 8);
    CHECK(T::sp(cpu) == STACK_TOP - 2);
    CHECK(stack_word(STACK_TOP - 2) == ORIGIN + 1);
    CHECK(T::flags(cpu) == ALL_FLAGS);
}


// --- PCHL ---

TEST_CASE("PCHL loads PC from HL", "[cpu][pchl]") {
    CPU cpu = make_cpu({0xE9}, ALL_FLAGS);     // PCHL
    T::h(cpu) = 0x12;
    T::l(cpu) = 0x34;
    T::sp(cpu) = STACK_TOP;

    CHECK(cpu.step(mem) == 5);
    CHECK(T::pc(cpu) == 0x1234);
    CHECK(T::sp(cpu) == STACK_TOP);
    CHECK(T::flags(cpu) == ALL_FLAGS);
}
