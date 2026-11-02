// Shared setup for CPU unit tests.

#ifndef CPU_TEST_HELPERS_HPP
#define CPU_TEST_HELPERS_HPP

#include <array>
#include <cstdint>
#include <initializer_list>

#include "../src/core/cpu.hpp"
#include "../src/core/flat_memory.hpp"

// Access to private CPU registers
struct CPUTestAccess {
    static uint8_t& a(CPU& cpu) { return cpu.a; }
    static uint8_t& b(CPU& cpu) { return cpu.b; }
    static uint8_t& c(CPU& cpu) { return cpu.c; }
    static uint8_t& d(CPU& cpu) { return cpu.d; }
    static uint8_t& e(CPU& cpu) { return cpu.e; }
    static uint8_t& h(CPU& cpu) { return cpu.h; }
    static uint8_t& l(CPU& cpu) { return cpu.l; }
    static uint16_t& pc(CPU& cpu) { return cpu.pc; }
    static uint16_t& sp(CPU& cpu) { return cpu.sp; }
    static uint8_t& flags(CPU& cpu) { return cpu.flags; }
    static bool& interrupts_enabled(CPU& cpu) { return cpu.interrupts_enabled; }
    static bool& ei_pending(CPU& cpu) { return cpu.ei_pending; }
    static bool& halted(CPU& cpu) { return cpu.halted; }
};
using T = CPUTestAccess;

// Flag masks
inline constexpr uint8_t CY = 0x01;
inline constexpr uint8_t P  = 0x04;
inline constexpr uint8_t AC = 0x10;
inline constexpr uint8_t Z  = 0x40;
inline constexpr uint8_t S  = 0x80;
inline constexpr uint8_t ALL_FLAGS = CY | P | AC | Z | S;

// Programs load here so a fall-through can't be confused with PC = 0
inline constexpr uint16_t ORIGIN = 0x0100;

// Memory shared by all CPU tests. make_cpu() resets it for each test.
inline FlatMemory mem;

// Reinitializes memory and loads program at ORIGIN
inline CPU make_cpu(std::initializer_list<uint8_t> program, uint8_t flags = 0) {
    mem = FlatMemory();
    uint16_t addr = ORIGIN;
    for (uint8_t byte : program)
        mem.write(addr++, byte);

    CPU cpu;
    T::pc(cpu) = ORIGIN;
    T::flags(cpu) = flags;
    return cpu;
}

// 3-bit condition codes shared by Jcc, Ccc, Rcc (opcode = base | cc << 3)
struct Condition {
    const char* name;
    uint8_t cc;
    uint8_t flag;
    bool if_set;    // true: condition holds when flag is set

    // Flags that make the condition true. Every other flag is in the
    // opposite state, so checking the wrong flag fails the test.
    uint8_t true_flags() const { return if_set ? flag : (ALL_FLAGS & ~flag); }
    uint8_t false_flags() const { return if_set ? (ALL_FLAGS & ~flag) : flag; }
};

inline const std::array<Condition, 8> CONDITIONS = {{
    {"NZ", 0, Z,  false},
    {"Z",  1, Z,  true},
    {"NC", 2, CY, false},
    {"C",  3, CY, true},
    {"PO", 4, P,  false},
    {"PE", 5, P,  true},
    {"P",  6, S,  false},
    {"M",  7, S,  true},
}};

#endif
