// Unit tests for NOP, HLT, EI, DI, and hardware interrupts.

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


// --- NOP ---

TEST_CASE("NOP only advances PC", "[cpu][control]") {
    CPU cpu = make_cpu({0x00}, ALL_FLAGS);     // NOP
    T::a(cpu) = 0x5A;
    T::sp(cpu) = STACK_TOP;

    CHECK(cpu.step(mem) == 4);
    CHECK(T::pc(cpu) == ORIGIN + 1);
    CHECK(T::a(cpu) == 0x5A);
    CHECK(T::sp(cpu) == STACK_TOP);
    CHECK(T::flags(cpu) == ALL_FLAGS);
}


// --- HLT ---

TEST_CASE("HLT halts the CPU", "[cpu][control]") {
    CPU cpu = make_cpu({0x76});                // HLT
    CHECK(cpu.step(mem) == 7);
    CHECK(T::halted(cpu));
    CHECK(T::pc(cpu) == ORIGIN + 1);
}

TEST_CASE("A halted CPU idles without fetching", "[cpu][control]") {
    CPU cpu = make_cpu({0x76, 0x3C});          // HLT; INR A
    cpu.step(mem);

    for (int i = 0; i < 3; i++) {
        CHECK(cpu.step(mem) == 4);
        CHECK(T::pc(cpu) == ORIGIN + 1);
    }
    CHECK(T::a(cpu) == 0x00);                  // INR A never ran
}


// --- EI / DI ---

TEST_CASE("DI disables interrupts", "[cpu][control]") {
    CPU cpu = make_cpu({0xF3});                // DI
    T::interrupts_enabled(cpu) = true;

    CHECK(cpu.step(mem) == 4);
    CHECK_FALSE(T::interrupts_enabled(cpu));
}

TEST_CASE("EI enables interrupts after the next instruction", "[cpu][control]") {
    CPU cpu = make_cpu({0xFB, 0x00});          // EI; NOP

    CHECK(cpu.step(mem) == 4);
    CHECK_FALSE(T::interrupts_enabled(cpu));   // still off right after EI
    CHECK(cpu.interrupt(mem, 1) == 0);         // so an interrupt here is ignored
    CHECK(T::pc(cpu) == ORIGIN + 1);

    cpu.step(mem);
    CHECK(T::interrupts_enabled(cpu));
}

TEST_CASE("DI right after EI leaves interrupts disabled", "[cpu][control]") {
    CPU cpu = make_cpu({0xFB, 0xF3});          // EI; DI
    cpu.step(mem);
    cpu.step(mem);
    CHECK_FALSE(T::interrupts_enabled(cpu));
    CHECK_FALSE(T::ei_pending(cpu));
}


// --- Interrupts ---

TEST_CASE("Interrupt pushes PC and jumps to rst_num * 8", "[cpu][interrupt]") {
    uint8_t n = GENERATE(range(0, 8));
    CAPTURE(n);

    CPU cpu = make_cpu({0x00}, ALL_FLAGS);
    T::interrupts_enabled(cpu) = true;
    T::sp(cpu) = STACK_TOP;

    CHECK(cpu.interrupt(mem, n) == 11);
    CHECK(T::pc(cpu) == n * 8);
    CHECK(T::sp(cpu) == STACK_TOP - 2);
    CHECK(stack_word(STACK_TOP - 2) == ORIGIN);
    CHECK_FALSE(T::interrupts_enabled(cpu));
    CHECK(T::flags(cpu) == ALL_FLAGS);
}

TEST_CASE("Interrupt is ignored while interrupts are disabled", "[cpu][interrupt]") {
    CPU cpu = make_cpu({0x00});
    T::sp(cpu) = STACK_TOP;

    CHECK(cpu.interrupt(mem, 2) == 0);
    CHECK(T::pc(cpu) == ORIGIN);
    CHECK(T::sp(cpu) == STACK_TOP);
}

TEST_CASE("Interrupt wakes a halted CPU", "[cpu][interrupt]") {
    CPU cpu = make_cpu({0xFB, 0x76});          // EI; HLT
    T::sp(cpu) = STACK_TOP;
    cpu.step(mem);
    cpu.step(mem);
    REQUIRE(T::halted(cpu));
    REQUIRE(T::interrupts_enabled(cpu));

    CHECK(cpu.interrupt(mem, 2) == 11);
    CHECK_FALSE(T::halted(cpu));
    CHECK(T::pc(cpu) == 0x0010);
    CHECK(stack_word(STACK_TOP - 2) == ORIGIN + 2);    // resumes after HLT
}

TEST_CASE("A halted CPU with interrupts disabled stays halted", "[cpu][interrupt]") {
    CPU cpu = make_cpu({0xF3, 0x76});          // DI; HLT
    cpu.step(mem);
    cpu.step(mem);

    CHECK(cpu.interrupt(mem, 2) == 0);
    CHECK(T::halted(cpu));
    CHECK(cpu.step(mem) == 4);
}
