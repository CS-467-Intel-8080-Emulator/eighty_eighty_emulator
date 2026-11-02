// Test harness for the 8080 spec suite (tests/spec).
//
// A TestMachine is a CPU wired to its own memory and a fake I/O device.
// Tests load a few bytes of 8080 code, run them, and compare the result with
// the Intel 8080 manual. The CPU is used only through its public interface;
// the one inside access is reading and setting registers.
//
//     TestMachine m;
//     CpuState start = CpuState::initial();
//
//     CHECK(m.run(start, {0x41}) == 5);    // MOV B,C: load, run, 5 cycles
//
//     CpuState expected = start;
//     expected.b = start.c;
//     expected.pc += 1;
//     CHECK(m.state() == expected);

#ifndef SPEC_TEST_MACHINE_HPP
#define SPEC_TEST_MACHINE_HPP

#include <cstdint>
#include <initializer_list>
#include <iomanip>
#include <ostream>
#include <utility>
#include <vector>

#include "core/cpu.hpp"
#include "core/io_bus.hpp"
#include "core/flat_memory.hpp"

namespace spec {

// Flag bits in the 8080 status byte: S Z 0 AC 0 P 1 CY
namespace flag {
    constexpr uint8_t S  = 0x80;   // sign
    constexpr uint8_t Z  = 0x40;   // zero
    constexpr uint8_t AC = 0x10;   // auxiliary carry
    constexpr uint8_t P  = 0x04;   // parity (even)
    constexpr uint8_t CY = 0x01;   // carry
    constexpr uint8_t ALL = S | Z | AC | P | CY;
}

// All 8080 registers at one moment
struct CpuState {
    uint8_t a = 0, b = 0, c = 0, d = 0, e = 0, h = 0, l = 0;
    uint8_t flags = 0;
    uint16_t sp = 0, pc = 0;

    uint16_t bc() const { return (uint16_t)(b << 8 | c); }
    uint16_t de() const { return (uint16_t)(d << 8 | e); }
    uint16_t hl() const { return (uint16_t)(h << 8 | l); }

    // A generic starting state, with flags as given (default clear).
    //
    // Tests must not depend on these specific values: expected values are
    // derived from the start state (expected.b = start.c), or the test sets
    // its own inputs first. The values can change, but initial() must return a
    // state where:
    //   - A, B, C, D, E, H, L all differ, so touching the wrong register is caught
    //   - PC's two bytes differ, so a return address with swapped bytes is caught
    //   - PC, SP, BC, DE and HL point at separate areas of memory, away from
    //     each other and from the fixed addresses tests use (0000-003F,
    //     3455-3457, FFFF)
    // These make the default state good at catching mistakes. They don't limit
    // the tests: a test may set equal registers or overlapping addresses on
    // purpose (e.g. ADD A, or overlap.cpp).
    static CpuState initial(uint8_t flags = 0) {
        CpuState s;
        s.flags = flags;
        s.a = 0x9C;
        s.b = 0x12; s.c = 0x34;
        s.d = 0x56; s.e = 0x78;
        s.h = 0x20; s.l = 0x50;
        s.sp = 0x2400;
        s.pc = 0x0840;
        return s;
    }

    // Compares the five defined flags only (bits 1, 3, 5 are unused)
    bool operator==(const CpuState& o) const {
        return a == o.a && b == o.b && c == o.c && d == o.d && e == o.e &&
               h == o.h && l == o.l && sp == o.sp && pc == o.pc &&
               (flags & flag::ALL) == (o.flags & flag::ALL);
    }
};

// Used by Catch2 in failure messages:
// A=9C B=12 C=34 D=56 E=78 H=20 L=50 SP=2400 PC=0840 F=00
inline std::ostream& operator<<(std::ostream& os, const CpuState& s) {
    std::ios_base::fmtflags saved_flags = os.flags();
    char saved_fill = os.fill();
    os << std::uppercase << std::hex << std::setfill('0')
       << "A=" << std::setw(2) << +s.a << " B=" << std::setw(2) << +s.b
       << " C=" << std::setw(2) << +s.c << " D=" << std::setw(2) << +s.d
       << " E=" << std::setw(2) << +s.e << " H=" << std::setw(2) << +s.h
       << " L=" << std::setw(2) << +s.l << " SP=" << std::setw(4) << s.sp
       << " PC=" << std::setw(4) << s.pc << " F=" << std::setw(2) << +(s.flags & flag::ALL);
    os.flags(saved_flags);
    os.fill(saved_fill);
    return os;
}

// Test I/O device (implements IOBus interface): IN returns preset values; every
// IN and OUT is recorded
class RecordingIO : public IOBus {
public:
    uint8_t inputs[256] = {};                          // value IN returns per port
    std::vector<uint8_t> reads;                        // ports read, in order
    std::vector<std::pair<uint8_t, uint8_t>> writes;   // (port, value), in order

    uint8_t in(uint8_t port) override {
        reads.push_back(port);
        return inputs[port];
    }

    void out(uint8_t port, uint8_t value) override {
        writes.emplace_back(port, value);
    }
};

}  // namespace spec

// CPU declares this struct a friend; here it only reads and sets registers.
struct CPUTestAccess {
    static spec::CpuState get(const CPU& cpu) {
        spec::CpuState s;
        s.a = cpu.a; s.b = cpu.b; s.c = cpu.c; s.d = cpu.d;
        s.e = cpu.e; s.h = cpu.h; s.l = cpu.l;
        s.flags = cpu.flags; s.sp = cpu.sp; s.pc = cpu.pc;
        return s;
    }

    static void set(CPU& cpu, const spec::CpuState& s) {
        cpu.a = s.a; cpu.b = s.b; cpu.c = s.c; cpu.d = s.d;
        cpu.e = s.e; cpu.h = s.h; cpu.l = s.l;
        cpu.flags = s.flags; cpu.sp = s.sp; cpu.pc = s.pc;
    }
};

namespace spec {

class TestMachine {
public:
    TestMachine() { cpu_.set_io(&io_); }

    // The CPU holds a pointer to io_, so a TestMachine can't be copied
    TestMachine(const TestMachine&) = delete;
    TestMachine& operator=(const TestMachine&) = delete;

    // Sets every register and loads the program at state.pc
    void start(const CpuState& state, std::initializer_list<uint8_t> program) {
        uint16_t addr = state.pc;
        for (uint8_t byte : program)
            memory_.write(addr++, byte);
        CPUTestAccess::set(cpu_, state);
    }

    // Runs one instruction; returns its cycle count
    uint32_t step() { return cpu_.step(memory_); }

    // start() followed by step(): runs a single instruction from a given state
    uint32_t run(const CpuState& state, std::initializer_list<uint8_t> program) {
        start(state, program);
        return step();
    }

    // Hardware interrupt supplying RST rst_num
    uint32_t interrupt(uint8_t rst_num) { return cpu_.interrupt(memory_, rst_num); }

    CpuState state() const { return CPUTestAccess::get(cpu_); }

    uint8_t read(uint16_t addr) const { return memory_.read(addr); }
    void write(uint16_t addr, uint8_t value) { memory_.write(addr, value); }

    // Writes a 16-bit value in 8080 byte order: low byte at addr, high byte at
    // addr + 1. For setting up memory only (e.g. a return address before RET).
    // Tests check results byte by byte with read(), so the expected byte order
    // is visible in each test instead of hidden here. There is deliberately no
    // read_word. This also intentionally doesn't use Memory's 16-bit helpers,
    // so that tests don't rely on the emulator's own code for write logic.
    void write_word(uint16_t addr, uint16_t value) {
        write(addr, (uint8_t)(value & 0xFF));
        write((uint16_t)(addr + 1), (uint8_t)(value >> 8));
    }

    RecordingIO& io() { return io_; }

private:
    FlatMemory memory_;
    RecordingIO io_;
    CPU cpu_;
};

}  // namespace spec

#endif
