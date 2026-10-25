// Unit tests for JMP and conditional jump (Jcc) instructions.

#include <cstdint>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include "cpu_test_helpers.hpp"


// --- JMP ---

TEST_CASE("JMP loads PC from the little-endian operand", "[cpu][jump]") {
    CPU cpu = make_cpu({0xC3, 0x34, 0x12});    // JMP 0x1234
    CHECK(cpu.step(mem) == 10);
    CHECK(T::pc(cpu) == 0x1234);
}

TEST_CASE("JMP does not change flags", "[cpu][jump]") {
    CPU cpu = make_cpu({0xC3, 0x34, 0x12}, ALL_FLAGS);
    cpu.step(mem);
    CHECK(T::flags(cpu) == ALL_FLAGS);
}

TEST_CASE("JMP can target the top and bottom of memory", "[cpu][jump]") {
    CPU cpu = make_cpu({0xC3, 0xFF, 0xFF});    // JMP 0xFFFF
    cpu.step(mem);
    CHECK(T::pc(cpu) == 0xFFFF);

    cpu = make_cpu({0xC3, 0x00, 0x00});        // JMP 0x0000
    cpu.step(mem);
    CHECK(T::pc(cpu) == 0x0000);
}

TEST_CASE("0xCB behaves as JMP", "[cpu][jump]") {
    CPU cpu = make_cpu({0xCB, 0x34, 0x12});    // Undocumented alias for JMP
    CHECK(cpu.step(mem) == 10);
    CHECK(T::pc(cpu) == 0x1234);
}


// --- Jcc ---

TEST_CASE("Jcc jumps only when its condition holds", "[cpu][jump]") {
    const Condition& cond = GENERATE(from_range(CONDITIONS));
    uint8_t opcode = 0xC2 | (cond.cc << 3);
    CAPTURE(cond.name);

    SECTION("condition true: jumps to the operand") {
        CPU cpu = make_cpu({opcode, 0xCD, 0xAB}, cond.true_flags());
        CHECK(cpu.step(mem) == 10);
        CHECK(T::pc(cpu) == 0xABCD);
        CHECK(T::flags(cpu) == cond.true_flags());
    }

    SECTION("condition false: skips the operand and falls through") {
        CPU cpu = make_cpu({opcode, 0xCD, 0xAB}, cond.false_flags());
        CHECK(cpu.step(mem) == 10);
        CHECK(T::pc(cpu) == ORIGIN + 3);
        CHECK(T::flags(cpu) == cond.false_flags());
    }
}
