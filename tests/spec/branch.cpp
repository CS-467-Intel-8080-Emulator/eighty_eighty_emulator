// Branch group (Intel 8080 manual): JMP, Jcc, CALL, Ccc, RET, Rcc, RST, PCHL.
// No branch instruction changes any flags.
//
// Conditions (cc) and the flag each one tests:
//   NZ: Z=0   Z: Z=1   NC: CY=0   C: CY=1   PO: P=0   PE: P=1   P: S=0   M: S=1
//
// Each conditional test runs twice: once with flags that make the condition
// true, once with flags that make it false. In each, every other flag is in the
// opposite state from the tested one, so testing the wrong flag fails.
//
// Stack layout: a push stores the high byte at SP-1 and the low byte at SP-2,
// then SP -= 2. Tests check each pushed byte separately.
// Return addresses and stack locations are written in terms of the start
// state (start.pc, start.sp), not fixed addresses.

#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "test_machine.hpp"

using namespace spec;
using namespace spec::flag;

namespace {

// Jcc addr: 3 bytes, 10 cycles whether or not it jumps
void check_jcc(uint8_t opcode, uint8_t true_flags, uint8_t false_flags) {
    TestMachine m;

    SECTION("condition true: jumps to the operand") {
        CpuState start = CpuState::initial(true_flags);
        CHECK(m.run(start, {opcode, 0x56, 0x34}) == 10);        // Jcc 3456
        CpuState expected = start;
        expected.pc = 0x3456;
        CHECK(m.state() == expected);
    }
    SECTION("condition false: skips the operand") {
        CpuState start = CpuState::initial(false_flags);
        CHECK(m.run(start, {opcode, 0x56, 0x34}) == 10);        // Jcc 3456
        CpuState expected = start;
        expected.pc += 3;
        CHECK(m.state() == expected);
    }
}

// Ccc addr: 3 bytes. Taken: push the return address, 17 cycles. Not taken: 11.
void check_ccc(uint8_t opcode, uint8_t true_flags, uint8_t false_flags) {
    TestMachine m;

    SECTION("condition true: pushes the return address and jumps") {
        CpuState start = CpuState::initial(true_flags);
        CHECK(m.run(start, {opcode, 0x56, 0x34}) == 17);        // Ccc 3456
        CpuState expected = start;
        expected.pc = 0x3456;
        expected.sp = start.sp - 2;
        CHECK(m.state() == expected);
        uint16_t ret = start.pc + 3;                    // return address
        CHECK(m.read(start.sp - 1) == (ret >> 8));      // high byte at SP-1
        CHECK(m.read(start.sp - 2) == (ret & 0xFF));    // low byte at SP-2
    }
    SECTION("condition false: skips the operand, stack untouched") {
        CpuState start = CpuState::initial(false_flags);
        CHECK(m.run(start, {opcode, 0x56, 0x34}) == 11);        // Ccc 3456
        CpuState expected = start;
        expected.pc += 3;
        CHECK(m.state() == expected);
        CHECK(m.read(start.sp - 1) == 0x00);
        CHECK(m.read(start.sp - 2) == 0x00);
    }
}

// Rcc: 1 byte. Taken: pop PC, 11 cycles. Not taken: 5 cycles.
// The stack holds return address 3456.
void check_rcc(uint8_t opcode, uint8_t true_flags, uint8_t false_flags) {
    TestMachine m;

    SECTION("condition true: pops the return address") {
        CpuState start = CpuState::initial(true_flags);
        m.write_word(start.sp, 0x3456);
        CHECK(m.run(start, {opcode}) == 11);
        CpuState expected = start;
        expected.pc = 0x3456;
        expected.sp = start.sp + 2;
        CHECK(m.state() == expected);
    }
    SECTION("condition false: falls through, SP unchanged") {
        CpuState start = CpuState::initial(false_flags);
        m.write_word(start.sp, 0x3456);
        CHECK(m.run(start, {opcode}) == 5);
        CpuState expected = start;
        expected.pc += 1;
        CHECK(m.state() == expected);
    }
}

// RST n: 1 byte, 11 cycles. Pushes the return address and jumps to n * 8.
void check_rst(uint8_t opcode, uint16_t target) {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {opcode}) == 11);

    CpuState expected = start;
    expected.pc = target;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
    uint16_t ret = start.pc + 1;                    // return address
    CHECK(m.read(start.sp - 1) == (ret >> 8));      // high byte at SP-1
    CHECK(m.read(start.sp - 2) == (ret & 0xFF));    // low byte at SP-2
}

}  // namespace


// ---------------------------------------------------------------------------
// JMP addr (0xC3): 3 bytes, address low byte first. 10 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("JMP addr (0xC3)", "[branch][jmp]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xC3, 0x56, 0x34}) == 10);              // JMP 3456

    CpuState expected = start;
    expected.pc = 0x3456;
    CHECK(m.state() == expected);
}

TEST_CASE("JMP can reach 0000 and FFFF", "[branch][jmp]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    CpuState expected = start;

    SECTION("JMP 0000") {
        m.run(start, {0xC3, 0x00, 0x00});
        expected.pc = 0x0000;
        CHECK(m.state() == expected);
    }
    SECTION("JMP FFFF") {
        m.run(start, {0xC3, 0xFF, 0xFF});
        expected.pc = 0xFFFF;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// Jcc addr: JNZ C2, JZ CA, JNC D2, JC DA, JPO E2, JPE EA, JP F2, JM FA
// ---------------------------------------------------------------------------

TEST_CASE("JNZ addr (0xC2)", "[branch][jcc]") { check_jcc(0xC2, S | AC | P | CY, Z); }
TEST_CASE("JZ addr (0xCA)",  "[branch][jcc]") { check_jcc(0xCA, Z, S | AC | P | CY); }
TEST_CASE("JNC addr (0xD2)", "[branch][jcc]") { check_jcc(0xD2, S | Z | AC | P, CY); }
TEST_CASE("JC addr (0xDA)",  "[branch][jcc]") { check_jcc(0xDA, CY, S | Z | AC | P); }
TEST_CASE("JPO addr (0xE2)", "[branch][jcc]") { check_jcc(0xE2, S | Z | AC | CY, P); }
TEST_CASE("JPE addr (0xEA)", "[branch][jcc]") { check_jcc(0xEA, P, S | Z | AC | CY); }
TEST_CASE("JP addr (0xF2)",  "[branch][jcc]") { check_jcc(0xF2, Z | AC | P | CY, S); }
TEST_CASE("JM addr (0xFA)",  "[branch][jcc]") { check_jcc(0xFA, S, Z | AC | P | CY); }


// ---------------------------------------------------------------------------
// CALL addr (0xCD): push PC of the next instruction, then jump. 17 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("CALL addr (0xCD)", "[branch][call]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xCD, 0x56, 0x34}) == 17);              // CALL 3456

    CpuState expected = start;
    expected.pc = 0x3456;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
    uint16_t ret = start.pc + 3;                    // return address
    CHECK(m.read(start.sp - 1) == (ret >> 8));      // high byte at SP-1
    CHECK(m.read(start.sp - 2) == (ret & 0xFF));    // low byte at SP-2
}

TEST_CASE("CALL with SP = 0001 wraps the push around 0000", "[branch][call]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.sp = 0x0001;

    m.run(start, {0xCD, 0x56, 0x34});                           // CALL 3456

    CpuState expected = start;
    expected.pc = 0x3456;
    expected.sp = 0xFFFF;
    CHECK(m.state() == expected);
    uint16_t ret = start.pc + 3;
    CHECK(m.read(0x0000) == (ret >> 8));          // high byte at SP-1 = 0000
    CHECK(m.read(0xFFFF) == (ret & 0xFF));        // low byte at SP-2 = FFFF
}


// ---------------------------------------------------------------------------
// Ccc addr: CNZ C4, CZ CC, CNC D4, CC DC, CPO E4, CPE EC, CP F4, CM FC
// ---------------------------------------------------------------------------

TEST_CASE("CNZ addr (0xC4)", "[branch][ccc]") { check_ccc(0xC4, S | AC | P | CY, Z); }
TEST_CASE("CZ addr (0xCC)",  "[branch][ccc]") { check_ccc(0xCC, Z, S | AC | P | CY); }
TEST_CASE("CNC addr (0xD4)", "[branch][ccc]") { check_ccc(0xD4, S | Z | AC | P, CY); }
TEST_CASE("CC addr (0xDC)",  "[branch][ccc]") { check_ccc(0xDC, CY, S | Z | AC | P); }
TEST_CASE("CPO addr (0xE4)", "[branch][ccc]") { check_ccc(0xE4, S | Z | AC | CY, P); }
TEST_CASE("CPE addr (0xEC)", "[branch][ccc]") { check_ccc(0xEC, P, S | Z | AC | CY); }
TEST_CASE("CP addr (0xF4)",  "[branch][ccc]") { check_ccc(0xF4, Z | AC | P | CY, S); }
TEST_CASE("CM addr (0xFC)",  "[branch][ccc]") { check_ccc(0xFC, S, Z | AC | P | CY); }


// ---------------------------------------------------------------------------
// RET (0xC9): pop PC. 10 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("RET (0xC9)", "[branch][ret]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    m.write_word(start.sp, 0x3456);

    CHECK(m.run(start, {0xC9}) == 10);

    CpuState expected = start;
    expected.pc = 0x3456;
    expected.sp = start.sp + 2;
    CHECK(m.state() == expected);
}

TEST_CASE("RET with SP = FFFF wraps the pop around 0000", "[branch][ret]") {
    TestMachine m;
    m.write(0xFFFF, 0x56);                // low byte
    m.write(0x0000, 0x34);                // high byte
    CpuState start = CpuState::initial(0);
    start.sp = 0xFFFF;

    m.run(start, {0xC9});

    CpuState expected = start;
    expected.pc = 0x3456;
    expected.sp = 0x0001;
    CHECK(m.state() == expected);
}

TEST_CASE("CALL then RET returns to the instruction after the CALL", "[branch][call][ret]") {
    TestMachine m;
    m.write(0x3456, 0xC9);                // RET at the subroutine
    CpuState start = CpuState::initial(ALL);
    m.start(start, {0xCD, 0x56, 0x34});   // CALL 3456

    m.step();                             // CALL
    m.step();                             // RET

    CpuState expected = start;
    expected.pc = start.pc + 3;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// Rcc: RNZ C0, RZ C8, RNC D0, RC D8, RPO E0, RPE E8, RP F0, RM F8
// ---------------------------------------------------------------------------

TEST_CASE("RNZ (0xC0)", "[branch][rcc]") { check_rcc(0xC0, S | AC | P | CY, Z); }
TEST_CASE("RZ (0xC8)",  "[branch][rcc]") { check_rcc(0xC8, Z, S | AC | P | CY); }
TEST_CASE("RNC (0xD0)", "[branch][rcc]") { check_rcc(0xD0, S | Z | AC | P, CY); }
TEST_CASE("RC (0xD8)",  "[branch][rcc]") { check_rcc(0xD8, CY, S | Z | AC | P); }
TEST_CASE("RPO (0xE0)", "[branch][rcc]") { check_rcc(0xE0, S | Z | AC | CY, P); }
TEST_CASE("RPE (0xE8)", "[branch][rcc]") { check_rcc(0xE8, P, S | Z | AC | CY); }
TEST_CASE("RP (0xF0)",  "[branch][rcc]") { check_rcc(0xF0, Z | AC | P | CY, S); }
TEST_CASE("RM (0xF8)",  "[branch][rcc]") { check_rcc(0xF8, S, Z | AC | P | CY); }


// ---------------------------------------------------------------------------
// RST n (0xC7 + 8n): call to address n * 8
// ---------------------------------------------------------------------------

TEST_CASE("RST 0 (0xC7)", "[branch][rst]") { check_rst(0xC7, 0x0000); }
TEST_CASE("RST 1 (0xCF)", "[branch][rst]") { check_rst(0xCF, 0x0008); }
TEST_CASE("RST 2 (0xD7)", "[branch][rst]") { check_rst(0xD7, 0x0010); }
TEST_CASE("RST 3 (0xDF)", "[branch][rst]") { check_rst(0xDF, 0x0018); }
TEST_CASE("RST 4 (0xE7)", "[branch][rst]") { check_rst(0xE7, 0x0020); }
TEST_CASE("RST 5 (0xEF)", "[branch][rst]") { check_rst(0xEF, 0x0028); }
TEST_CASE("RST 6 (0xF7)", "[branch][rst]") { check_rst(0xF7, 0x0030); }
TEST_CASE("RST 7 (0xFF)", "[branch][rst]") { check_rst(0xFF, 0x0038); }


// ---------------------------------------------------------------------------
// PCHL (0xE9): PC = HL. 1 byte. 5 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("PCHL (0xE9)", "[branch][pchl]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xE9}) == 5);

    CpuState expected = start;
    expected.pc = start.hl();
    CHECK(m.state() == expected);
}
