// Overlapping memory: code, data and stack sharing the same bytes.
//
// The other files keep the program, the stack and any data apart, so each
// instruction's effect is easy to see. Real programs don't always do that, so
// these tests overlap them on purpose:
//   - an instruction writes over the next instruction (self-modifying code),
//     and the new bytes are what runs
//   - PUSH writes over code that runs afterwards
//   - an instruction reads its own bytes as data
//
// An emulator that fetches every byte from memory as it runs passes these
// without special handling. They catch one that caches or prefetches
// instruction bytes.
//
// Programs load at 0100, except the LHLD test, which loads at 0110 so its own
// operand bytes form the address it reads. Each test sets every register and
// address it uses.

#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "test_machine.hpp"

using namespace spec;
using namespace spec::flag;

TEST_CASE("MOV M,A writes over the next opcode, and the new opcode runs", "[overlap]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.pc = 0x0100;
    start.h = 0x01; start.l = 0x01;        // HL points at the next instruction
    start.a = 0x3C;                        // 3C is the opcode for INR A

    m.start(start, {
        0x77,                              // 0100  MOV M,A
        0x00,                              // 0101  NOP, replaced by INR A
    });

    CHECK(m.step() == 7);                  // MOV M,A
    CHECK(m.read(0x0101) == 0x3C);
    CHECK(m.step() == 5);                  // INR A, not NOP (4)

    CpuState expected = start;
    expected.a = 0x3D;
    expected.flags = 0;
    expected.pc = 0x0102;
    CHECK(m.state() == expected);
}

TEST_CASE("MVI M writes over the next instruction's data byte", "[overlap]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.pc = 0x0100;
    start.h = 0x01; start.l = 0x03;        // HL points at MVI A's data byte
    start.a = 0x11;

    m.start(start, {
        0x36, 0x55,                        // 0100  MVI M,55
        0x3E, 0x00,                        // 0102  MVI A,00, data replaced by 55
    });

    CHECK(m.step() == 10);                 // MVI M
    CHECK(m.read(0x0103) == 0x55);
    CHECK(m.step() == 7);                  // MVI A

    CpuState expected = start;
    expected.a = 0x55;
    expected.pc = 0x0104;
    CHECK(m.state() == expected);
}

TEST_CASE("PUSH writes over the instructions that run next", "[overlap]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.pc = 0x0100;
    start.sp = 0x0103;                     // the stack grows down into the code
    start.b = 0x00;                        // pushed to 0102: NOP
    start.c = 0x3C;                        // pushed to 0101: INR A
    start.a = 0x41;

    m.start(start, {
        0xC5,                              // 0100  PUSH B
        0x76,                              // 0101  HLT, replaced by INR A
        0x76,                              // 0102  HLT, replaced by NOP
    });

    CHECK(m.step() == 11);                 // PUSH B
    CHECK(m.read(0x0102) == 0x00);         // B (high byte) at SP-1
    CHECK(m.read(0x0101) == 0x3C);         // C (low byte) at SP-2
    CHECK(m.step() == 5);                  // INR A, not HLT (7)
    CHECK(m.step() == 4);                  // NOP

    CpuState expected = start;
    expected.a = 0x42;
    expected.flags = P;
    expected.sp = 0x0101;
    expected.pc = 0x0103;
    CHECK(m.state() == expected);
}

TEST_CASE("MOV A,M with HL pointing at itself reads its own opcode", "[overlap]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.pc = 0x0100;
    start.h = 0x01; start.l = 0x00;        // HL = PC
    start.a = 0x00;

    CHECK(m.run(start, {0x7E}) == 7);      // 0100  MOV A,M

    CpuState expected = start;
    expected.a = 0x7E;
    expected.pc = 0x0101;
    CHECK(m.state() == expected);
}

TEST_CASE("LHLD reads its own address bytes", "[overlap]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.pc = 0x0110;
    start.h = 0x20; start.l = 0x50;

    // LHLD 0111: the address is the instruction's own operand, so
    // L = (0111) = 11 and H = (0112) = 01.
    CHECK(m.run(start, {0x2A, 0x11, 0x01}) == 16);   // 0110  LHLD 0111

    CpuState expected = start;
    expected.h = 0x01; expected.l = 0x11;
    expected.pc = 0x0113;
    CHECK(m.state() == expected);
}
