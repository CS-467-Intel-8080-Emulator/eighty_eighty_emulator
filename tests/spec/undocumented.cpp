// Undocumented opcodes. The Intel 8080 manual leaves these 12 opcodes out,
// but the real chip runs each one as a copy of a documented instruction
// (mapping as in https://pastraiser.com/cpu/i8080/i8080_opcodes.html):
//   08 10 18 20 28 30 38 -> NOP
//   CB                   -> JMP addr
//   D9                   -> RET
//   DD ED FD             -> CALL addr
//
// Expected values are written in terms of the start state (start.pc,
// start.sp), never the specific values CpuState::initial() uses.

#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "test_machine.hpp"

using namespace spec;
using namespace spec::flag;

namespace {

// NOP copy: 1 byte, 4 cycles, only PC changes
void check_nop(uint8_t opcode) {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {opcode}) == 4);

    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

// CALL copy: 3 bytes, 17 cycles, pushes the return address
void check_call(uint8_t opcode) {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {opcode, 0x56, 0x34}) == 17);            // CALL 3456

    CpuState expected = start;
    expected.pc = 0x3456;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
    uint16_t ret = start.pc + 3;                    // return address
    CHECK(m.read(start.sp - 1) == (ret >> 8));      // high byte at SP-1
    CHECK(m.read(start.sp - 2) == (ret & 0xFF));    // low byte at SP-2
}

}  // namespace


// ---------------------------------------------------------------------------
// NOP copies
// ---------------------------------------------------------------------------

TEST_CASE("Undocumented NOP (0x08)", "[undocumented][nop]") { check_nop(0x08); }
TEST_CASE("Undocumented NOP (0x10)", "[undocumented][nop]") { check_nop(0x10); }
TEST_CASE("Undocumented NOP (0x18)", "[undocumented][nop]") { check_nop(0x18); }
TEST_CASE("Undocumented NOP (0x20)", "[undocumented][nop]") { check_nop(0x20); }
TEST_CASE("Undocumented NOP (0x28)", "[undocumented][nop]") { check_nop(0x28); }
TEST_CASE("Undocumented NOP (0x30)", "[undocumented][nop]") { check_nop(0x30); }
TEST_CASE("Undocumented NOP (0x38)", "[undocumented][nop]") { check_nop(0x38); }

TEST_CASE("Undocumented NOP does not stop the CPU", "[undocumented][nop]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    m.start(start, {0x08, 0x06, 0xAA});   // NOP copy; MVI B,AA

    m.step();
    m.step();

    CpuState expected = start;
    expected.pc += 3;
    expected.b = 0xAA;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// JMP copy (0xCB): 3 bytes, 10 cycles
// ---------------------------------------------------------------------------

TEST_CASE("Undocumented JMP (0xCB)", "[undocumented][jmp]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xCB, 0x56, 0x34}) == 10);              // JMP 3456

    CpuState expected = start;
    expected.pc = 0x3456;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// RET copy (0xD9): 1 byte, 10 cycles
// ---------------------------------------------------------------------------

TEST_CASE("Undocumented RET (0xD9)", "[undocumented][ret]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    m.write_word(start.sp, 0x3456);       // return address on the stack

    CHECK(m.run(start, {0xD9}) == 10);

    CpuState expected = start;
    expected.pc = 0x3456;
    expected.sp = start.sp + 2;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// CALL copies (0xDD, 0xED, 0xFD)
// ---------------------------------------------------------------------------

TEST_CASE("Undocumented CALL (0xDD)", "[undocumented][call]") { check_call(0xDD); }
TEST_CASE("Undocumented CALL (0xED)", "[undocumented][call]") { check_call(0xED); }
TEST_CASE("Undocumented CALL (0xFD)", "[undocumented][call]") { check_call(0xFD); }
