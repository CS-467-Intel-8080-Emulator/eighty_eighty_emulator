// Arithmetic group (Intel 8080 manual): ADD, ADC, SUB, SBB, ADI, ACI, SUI, SBI,
// INR, DCR, INX, DCX, DAD, DAA.
//
// Flag rules from the manual:
//   ADD/ADC/SUB/SBB and immediates: S, Z, AC, P, CY all updated from the result
//   INR/DCR: S, Z, AC, P updated; CY unchanged
//   INX/DCX: no flags change
//   DAD:     only CY changes
//   DAA:     S, Z, AC, P updated; CY can be set but is never cleared
// Subtraction is A + (NOT operand) + 1 (SBB/SBI with a borrow in: + 0), with AC
// taken from the carry out of bit 3 of that addition. So AC is set when there
// is NO borrow out of bit 3 (low nibble of A >= low nibble of the operand),
// and CY is set when there IS a borrow (A < operand). See README assumption 4.
//
// Why operands are set explicitly instead of derived from CpuState::initial():
// unlike data moves (MOV B,C just expects start.c), an ALU result comes with
// flags. Deriving them in the test (Z, S, parity, AC, CY) would mean writing a
// second copy of the flag logic being tested, and a mistake shared by both
// copies would pass. So each test sets fixed inputs and hard-codes the result
// and flags as a hand-checked example. The inputs are also picked on purpose
// to hit specific cases (zero, carry, borrow, AC); taking them from initial()
// would let a change there silently drop those cases. Memory operands (M) are
// written at whatever HL holds.
//
// Tests marked "(ALP p. N)" are worked examples from Intel's 8080 Assembly
// Language Programming Manual (1975, Rev. B), printed page N.

#include <cstdint>
#include <initializer_list>

#include <catch2/catch_test_macros.hpp>

#include "test_machine.hpp"

using namespace spec;
using namespace spec::flag;

// ---------------------------------------------------------------------------
// ADD r / ADD M (0x80-0x87): A = A + r. 4 cycles (M: 7).
// Starts with all flags set: ADD ignores the incoming carry, and every flag
// that should be clear must be cleared.
// ---------------------------------------------------------------------------

TEST_CASE("ADD r (0x80-0x87)", "[arithmetic][add]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("ADD B (0x80): 9C + 12 = AE") {
        CHECK(m.run(start, {0x80}) == 4);
        expected.a = 0xAE; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("ADD C (0x81): 9C + 34 = D0") {
        CHECK(m.run(start, {0x81}) == 4);
        expected.a = 0xD0; expected.flags = S | AC;
        CHECK(m.state() == expected);
    }
    SECTION("ADD D (0x82): 9C + 56 = F2") {
        CHECK(m.run(start, {0x82}) == 4);
        expected.a = 0xF2; expected.flags = S | AC;
        CHECK(m.state() == expected);
    }
    SECTION("ADD E (0x83): 9C + 78 = 114 (carry)") {
        CHECK(m.run(start, {0x83}) == 4);
        expected.a = 0x14; expected.flags = AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("ADD H (0x84): 9C + 20 = BC") {
        CHECK(m.run(start, {0x84}) == 4);
        expected.a = 0xBC; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("ADD L (0x85): 9C + 50 = EC") {
        CHECK(m.run(start, {0x85}) == 4);
        expected.a = 0xEC; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("ADD M (0x86): 9C + 64 = 100 (zero, carry)") {
        m.write(start.hl(), 0x64);
        CHECK(m.run(start, {0x86}) == 7);
        expected.a = 0x00; expected.flags = Z | AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("ADD A (0x87): 9C + 9C = 138 (carry)") {
        CHECK(m.run(start, {0x87}) == 4);
        expected.a = 0x38; expected.flags = AC | CY;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("ADD example: 6C + 2E = 9A (ALP p. 18)", "[arithmetic][add]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.a = 0x6C;
    start.d = 0x2E;

    m.run(start, {0x82});                                  // ADD D

    CpuState expected = start;
    expected.a = 0x9A;
    expected.flags = S | AC | P;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

// The manual's condition-bit chapter illustrates the flags with three plain
// additions. The tests run them as ADD B.
TEST_CASE("Condition-bit examples (ALP pp. 5-6)", "[arithmetic][add]") {
    TestMachine m;

    SECTION("AE + 74 = 22 with carry out: CY set (ALP p. 5; AC, P from the general rules)") {
        CpuState start = CpuState::initial(S | Z);
        start.a = 0xAE;
        start.b = 0x74;
        m.run(start, {0x80});                              // ADD B
        CpuState expected = start;
        expected.a = 0x22;
        expected.flags = AC | P | CY;
        expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("2E + 74 = A2: CY clear, AC set (ALP p. 6; S, P from the general rules)") {
        CpuState start = CpuState::initial(Z | P | CY);
        start.a = 0x2E;
        start.b = 0x74;
        m.run(start, {0x80});                              // ADD B
        CpuState expected = start;
        expected.a = 0xA2;
        expected.flags = S | AC;
        expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("A7 + 59 = 00 with carry out: Z set (ALP p. 6; AC, P, CY from the general rules)") {
        CpuState start = CpuState::initial(S);
        start.a = 0xA7;
        start.b = 0x59;
        m.run(start, {0x80});                              // ADD B
        CpuState expected = start;
        expected.a = 0x00;
        expected.flags = Z | AC | P | CY;
        expected.pc += 1;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// ADC r / ADC M (0x88-0x8F): A = A + r + CY. 4 cycles (M: 7).
// Per-register sections start with all flags set, so the carry-in is 1.
// ---------------------------------------------------------------------------

TEST_CASE("ADC r (0x88-0x8F)", "[arithmetic][adc]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("ADC B (0x88): 9C + 12 + 1 = AF") {
        CHECK(m.run(start, {0x88}) == 4);
        expected.a = 0xAF; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("ADC C (0x89): 9C + 34 + 1 = D1") {
        CHECK(m.run(start, {0x89}) == 4);
        expected.a = 0xD1; expected.flags = S | AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("ADC D (0x8A): 9C + 56 + 1 = F3") {
        CHECK(m.run(start, {0x8A}) == 4);
        expected.a = 0xF3; expected.flags = S | AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("ADC E (0x8B): 9C + 78 + 1 = 115 (carry)") {
        CHECK(m.run(start, {0x8B}) == 4);
        expected.a = 0x15; expected.flags = AC | CY;
        CHECK(m.state() == expected);
    }
    SECTION("ADC H (0x8C): 9C + 20 + 1 = BD") {
        CHECK(m.run(start, {0x8C}) == 4);
        expected.a = 0xBD; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("ADC L (0x8D): 9C + 50 + 1 = ED") {
        CHECK(m.run(start, {0x8D}) == 4);
        expected.a = 0xED; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("ADC M (0x8E): 9C + 63 + 1 = 100 (zero, carry)") {
        m.write(start.hl(), 0x63);
        CHECK(m.run(start, {0x8E}) == 7);
        expected.a = 0x00; expected.flags = Z | AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("ADC A (0x8F): 9C + 9C + 1 = 139 (carry)") {
        CHECK(m.run(start, {0x8F}) == 4);
        expected.a = 0x39; expected.flags = AC | P | CY;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("ADC example: 42 + 3D with and without carry (ALP p. 18)", "[arithmetic][adc]") {
    TestMachine m;

    SECTION("carry clear: 42 + 3D + 0 = 7F") {
        CpuState start = CpuState::initial(0);
        start.a = 0x42;
        start.c = 0x3D;
        m.run(start, {0x89});                              // ADC C
        CpuState expected = start;
        expected.a = 0x7F;
        expected.flags = 0;
        expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("carry set: 42 + 3D + 1 = 80") {
        CpuState start = CpuState::initial(CY);
        start.a = 0x42;
        start.c = 0x3D;
        m.run(start, {0x89});                              // ADC C
        CpuState expected = start;
        expected.a = 0x80;
        expected.flags = S | AC;
        expected.pc += 1;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// SUB r / SUB M (0x90-0x97): A = A - r. 4 cycles (M: 7).
// Starts with all flags set: SUB ignores the incoming carry.
// ---------------------------------------------------------------------------

TEST_CASE("SUB r (0x90-0x97)", "[arithmetic][sub]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("SUB B (0x90): 9C - 12 = 8A") {
        CHECK(m.run(start, {0x90}) == 4);
        expected.a = 0x8A; expected.flags = S | AC;
        CHECK(m.state() == expected);
    }
    SECTION("SUB C (0x91): 9C - 34 = 68") {
        CHECK(m.run(start, {0x91}) == 4);
        expected.a = 0x68; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("SUB D (0x92): 9C - 56 = 46") {
        CHECK(m.run(start, {0x92}) == 4);
        expected.a = 0x46; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("SUB E (0x93): 9C - 78 = 24") {
        CHECK(m.run(start, {0x93}) == 4);
        expected.a = 0x24; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("SUB H (0x94): 9C - 20 = 7C") {
        CHECK(m.run(start, {0x94}) == 4);
        expected.a = 0x7C; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("SUB L (0x95): 9C - 50 = 4C") {
        CHECK(m.run(start, {0x95}) == 4);
        expected.a = 0x4C; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("SUB M (0x96): 9C - A5 = F7 (borrow)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x96}) == 7);
        expected.a = 0xF7; expected.flags = S | AC | CY;
        CHECK(m.state() == expected);
    }
    SECTION("SUB A (0x97): A - A = 0") {
        CHECK(m.run(start, {0x97}) == 4);
        expected.a = 0x00; expected.flags = Z | AC | P;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("SUB A example: 3E - 3E = 0, CY clear, AC set (ALP p. 18)", "[arithmetic][sub]") {
    TestMachine m;
    CpuState start = CpuState::initial(CY | S);
    start.a = 0x3E;

    m.run(start, {0x97});                                  // SUB A

    CpuState expected = start;
    expected.a = 0x00;
    expected.flags = Z | AC | P;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("SUB with a borrow out of bit 3 clears AC: 10 - 01 = 0F", "[arithmetic][sub]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x10;
    start.b = 0x01;

    m.run(start, {0x90});                                  // SUB B

    CpuState expected = start;
    expected.a = 0x0F;
    expected.flags = P;
    expected.pc += 1;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// SBB r / SBB M (0x98-0x9F): A = A - r - CY. 4 cycles (M: 7).
// Per-register sections start with all flags set, so the borrow-in is 1.
// With a borrow-in, AC is set when low nibble of A > low nibble of r.
// ---------------------------------------------------------------------------

TEST_CASE("SBB r (0x98-0x9F)", "[arithmetic][sbb]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("SBB B (0x98): 9C - 12 - 1 = 89") {
        CHECK(m.run(start, {0x98}) == 4);
        expected.a = 0x89; expected.flags = S | AC;
        CHECK(m.state() == expected);
    }
    SECTION("SBB C (0x99): 9C - 34 - 1 = 67") {
        CHECK(m.run(start, {0x99}) == 4);
        expected.a = 0x67; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("SBB D (0x9A): 9C - 56 - 1 = 45") {
        CHECK(m.run(start, {0x9A}) == 4);
        expected.a = 0x45; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("SBB E (0x9B): 9C - 78 - 1 = 23") {
        CHECK(m.run(start, {0x9B}) == 4);
        expected.a = 0x23; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("SBB H (0x9C): 9C - 20 - 1 = 7B") {
        CHECK(m.run(start, {0x9C}) == 4);
        expected.a = 0x7B; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("SBB L (0x9D): 9C - 50 - 1 = 4B") {
        CHECK(m.run(start, {0x9D}) == 4);
        expected.a = 0x4B; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("SBB M (0x9E): 9C - A5 - 1 = F6 (borrow)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x9E}) == 7);
        expected.a = 0xF6; expected.flags = S | AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("SBB A (0x9F): 9C - 9C - 1 = FF (borrow)") {
        CHECK(m.run(start, {0x9F}) == 4);
        expected.a = 0xFF; expected.flags = S | P | CY;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("SBB example: 04 - 02 - 1 = 01 (ALP p. 19)", "[arithmetic][sbb]") {
    TestMachine m;
    CpuState start = CpuState::initial(CY);
    start.a = 0x04;
    start.l = 0x02;

    m.run(start, {0x9D});                                  // SBB L

    CpuState expected = start;
    expected.a = 0x01;
    expected.flags = AC;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("SBB with carry clear subtracts like SUB: 9C - 12 = 8A", "[arithmetic][sbb]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.a = 0x9C;
    start.b = 0x12;

    m.run(start, {0x98});                                  // SBB B

    CpuState expected = start;
    expected.a = 0x8A;
    expected.flags = S | AC;
    expected.pc += 1;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// Immediates: ADI (0xC6), ACI (0xCE), SUI (0xD6), SBI (0xDE)
// 2 bytes. 7 cycles. Same flag rules as the register forms.
// ---------------------------------------------------------------------------

TEST_CASE("ADI data (0xC6)", "[arithmetic][adi]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 2;

    SECTION("9C + 12 = AE") {
        CHECK(m.run(start, {0xC6, 0x12}) == 7);
        expected.a = 0xAE; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("9C + 64 = 100 (zero, carry)") {
        CHECK(m.run(start, {0xC6, 0x64}) == 7);
        expected.a = 0x00; expected.flags = Z | AC | P | CY;
        CHECK(m.state() == expected);
    }
}

// ALP p. 27: MVI A,20 then ADI 66 then ADI -66 (66 decimal = 42H, -66 = BEH).
TEST_CASE("ADI examples: 14 + 42 = 56, then 56 + BE = 14 (ALP p. 27)", "[arithmetic][adi]") {
    TestMachine m;

    SECTION("ADI 42: 14 + 42 = 56, only P set") {
        CpuState start = CpuState::initial(S | Z | AC | CY);
        start.a = 0x14;
        m.run(start, {0xC6, 0x42});
        CpuState expected = start;
        expected.a = 0x56;
        expected.flags = P;
        expected.pc += 2;
        CHECK(m.state() == expected);
    }
    SECTION("ADI BE: 56 + BE = 14, CY AC P set, Z S clear") {
        CpuState start = CpuState::initial(S | Z);
        start.a = 0x56;
        m.run(start, {0xC6, 0xBE});
        CpuState expected = start;
        expected.a = 0x14;
        expected.flags = AC | P | CY;
        expected.pc += 2;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("ACI data (0xCE)", "[arithmetic][aci]") {
    TestMachine m;

    SECTION("carry set: 9C + 63 + 1 = 100 (zero, carry)") {
        CpuState start = CpuState::initial(CY);
        start.a = 0x9C;
        CHECK(m.run(start, {0xCE, 0x63}) == 7);
        CpuState expected = start;
        expected.a = 0x00; expected.flags = Z | AC | P | CY; expected.pc += 2;
        CHECK(m.state() == expected);
    }
    SECTION("carry clear: 9C + 63 + 0 = FF") {
        CpuState start = CpuState::initial(0);
        start.a = 0x9C;
        CHECK(m.run(start, {0xCE, 0x63}) == 7);
        CpuState expected = start;
        expected.a = 0xFF; expected.flags = S | P; expected.pc += 2;
        CHECK(m.state() == expected);
    }
}

// ALP p. 27: MVI A,56H then ACI -66 then ACI 66 (-66 = BEH, 66 = 42H).
TEST_CASE("ACI examples: 56 + BE + 0 = 14, then 14 + 42 + 1 = 57 (ALP p. 27)", "[arithmetic][aci]") {
    TestMachine m;

    SECTION("ACI BE, carry clear: 56 + BE + 0 = 14, CY AC P set") {
        CpuState start = CpuState::initial(S | Z);
        start.a = 0x56;
        m.run(start, {0xCE, 0xBE});
        CpuState expected = start;
        expected.a = 0x14;
        expected.flags = AC | P | CY;
        expected.pc += 2;
        CHECK(m.state() == expected);
    }
    SECTION("ACI 42, carry set: 14 + 42 + 1 = 57 (flags not stated)") {
        CpuState start = CpuState::initial(ALL);
        start.a = 0x14;
        m.run(start, {0xCE, 0x42});
        CpuState expected = start;
        expected.a = 0x57;
        expected.flags = 0;
        expected.pc += 2;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("SUI data (0xD6)", "[arithmetic][sui]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 2;

    SECTION("9C - A5 = F7 (borrow)") {
        CHECK(m.run(start, {0xD6, 0xA5}) == 7);
        expected.a = 0xF7; expected.flags = S | AC | CY;
        CHECK(m.state() == expected);
    }
    SECTION("9C - 9C = 0") {
        CHECK(m.run(start, {0xD6, 0x9C}) == 7);
        expected.a = 0x00; expected.flags = Z | AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("SUI example: 00 - 01 = FF, CY S P set, Z AC clear (ALP pp. 27-28)") {
        start.a = 0x00;
        CHECK(m.run(start, {0xD6, 0x01}) == 7);
        expected.a = 0xFF; expected.flags = S | P | CY;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("SBI data (0xDE)", "[arithmetic][sbi]") {
    TestMachine m;

    SECTION("carry set: 9C - A5 - 1 = F6 (borrow)") {
        CpuState start = CpuState::initial(CY);
        start.a = 0x9C;
        CHECK(m.run(start, {0xDE, 0xA5}) == 7);
        CpuState expected = start;
        expected.a = 0xF6; expected.flags = S | AC | P | CY; expected.pc += 2;
        CHECK(m.state() == expected);
    }
    SECTION("carry clear: 9C - A5 - 0 = F7 (borrow)") {
        CpuState start = CpuState::initial(0);
        start.a = 0x9C;
        CHECK(m.run(start, {0xDE, 0xA5}) == 7);
        CpuState expected = start;
        expected.a = 0xF7; expected.flags = S | AC | CY; expected.pc += 2;
        CHECK(m.state() == expected);
    }
    SECTION("SBI example, carry clear: 00 - 01 - 0 = FF, CY S P set, Z AC clear (ALP p. 28)") {
        CpuState start = CpuState::initial(Z | AC);
        start.a = 0x00;
        CHECK(m.run(start, {0xDE, 0x01}) == 7);
        CpuState expected = start;
        expected.a = 0xFF; expected.flags = S | P | CY; expected.pc += 2;
        CHECK(m.state() == expected);
    }
    SECTION("SBI example, carry set: 00 - 01 - 1 = FE, CY S set, Z P AC clear (ALP p. 28)") {
        CpuState start = CpuState::initial(Z | AC | P | CY);
        start.a = 0x00;
        CHECK(m.run(start, {0xDE, 0x01}) == 7);
        CpuState expected = start;
        expected.a = 0xFE; expected.flags = S | CY; expected.pc += 2;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// INR r / INR M (0x04, 0x0C, ... 0x3C): r = r + 1. 5 cycles (M: 10).
// AC is set when the low nibble of the result is 0 (carry out of bit 3).
// CY is never changed: per-register sections start with all flags set, so CY
// must stay 1; the edge-case sections start clear, so CY must stay 0.
// ---------------------------------------------------------------------------

TEST_CASE("INR r (0x04-0x3C)", "[arithmetic][inr]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("INR B (0x04): 12 -> 13") {
        CHECK(m.run(start, {0x04}) == 5);
        expected.b = 0x13; expected.flags = CY;
        CHECK(m.state() == expected);
    }
    SECTION("INR C (0x0C): 34 -> 35") {
        CHECK(m.run(start, {0x0C}) == 5);
        expected.c = 0x35; expected.flags = P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("INR D (0x14): 56 -> 57") {
        CHECK(m.run(start, {0x14}) == 5);
        expected.d = 0x57; expected.flags = CY;
        CHECK(m.state() == expected);
    }
    SECTION("INR E (0x1C): 78 -> 79") {
        CHECK(m.run(start, {0x1C}) == 5);
        expected.e = 0x79; expected.flags = CY;
        CHECK(m.state() == expected);
    }
    SECTION("INR H (0x24): 20 -> 21") {
        CHECK(m.run(start, {0x24}) == 5);
        expected.h = 0x21; expected.flags = P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("INR L (0x2C): 50 -> 51") {
        CHECK(m.run(start, {0x2C}) == 5);
        expected.l = 0x51; expected.flags = CY;
        CHECK(m.state() == expected);
    }
    SECTION("INR M (0x34): FF -> 00") {
        m.write(start.hl(), 0xFF);
        CHECK(m.run(start, {0x34}) == 10);
        CHECK(m.read(start.hl()) == 0x00);
        expected.flags = Z | AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("INR A (0x3C): 9C -> 9D") {
        CHECK(m.run(start, {0x3C}) == 5);
        expected.a = 0x9D; expected.flags = S | CY;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("INR example: C 99 -> 9A (ALP p. 15; flags not stated)", "[arithmetic][inr]") {
    TestMachine m;
    CpuState start = CpuState::initial(Z | AC | CY);
    start.c = 0x99;

    m.run(start, {0x0C});                                  // INR C

    CpuState expected = start;
    expected.c = 0x9A;
    expected.flags = S | P | CY;                           // CY unchanged
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("INR edge cases (carry stays clear)", "[arithmetic][inr]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("0F -> 10 sets AC") {
        start.b = 0x0F;
        m.run(start, {0x04});                              // INR B
        expected.b = 0x10; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("7F -> 80 sets S") {
        start.b = 0x7F;
        m.run(start, {0x04});                              // INR B
        expected.b = 0x80; expected.flags = S | AC;
        CHECK(m.state() == expected);
    }
    SECTION("FF -> 00 wraps, sets Z, leaves CY clear") {
        start.b = 0xFF;
        m.run(start, {0x04});                              // INR B
        expected.b = 0x00; expected.flags = Z | AC | P;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// DCR r / DCR M (0x05, 0x0D, ... 0x3D): r = r - 1. 5 cycles (M: 10).
// AC is set unless the low nibble of the result is F (no borrow out of bit 3).
// CY is never changed.
// ---------------------------------------------------------------------------

TEST_CASE("DCR r (0x05-0x3D)", "[arithmetic][dcr]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("DCR B (0x05): 12 -> 11") {
        CHECK(m.run(start, {0x05}) == 5);
        expected.b = 0x11; expected.flags = AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("DCR C (0x0D): 34 -> 33") {
        CHECK(m.run(start, {0x0D}) == 5);
        expected.c = 0x33; expected.flags = AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("DCR D (0x15): 56 -> 55") {
        CHECK(m.run(start, {0x15}) == 5);
        expected.d = 0x55; expected.flags = AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("DCR E (0x1D): 78 -> 77") {
        CHECK(m.run(start, {0x1D}) == 5);
        expected.e = 0x77; expected.flags = AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("DCR H (0x25): 20 -> 1F") {
        CHECK(m.run(start, {0x25}) == 5);
        expected.h = 0x1F; expected.flags = CY;
        CHECK(m.state() == expected);
    }
    SECTION("DCR L (0x2D): 50 -> 4F") {
        CHECK(m.run(start, {0x2D}) == 5);
        expected.l = 0x4F; expected.flags = CY;
        CHECK(m.state() == expected);
    }
    SECTION("DCR M (0x35): 01 -> 00") {
        m.write(start.hl(), 0x01);
        CHECK(m.run(start, {0x35}) == 10);
        CHECK(m.read(start.hl()) == 0x00);
        expected.flags = Z | AC | P | CY;
        CHECK(m.state() == expected);
    }
    SECTION("DCR A (0x3D): 9C -> 9B") {
        CHECK(m.run(start, {0x3D}) == 5);
        expected.a = 0x9B; expected.flags = S | AC | CY;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("DCR edge cases (carry stays clear)", "[arithmetic][dcr]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("DCR M example: 40 -> 3F at 3A7C (ALP p. 15; flags not stated)") {
        start.h = 0x3A; start.l = 0x7C;
        expected.h = 0x3A; expected.l = 0x7C;
        m.write(0x3A7C, 0x40);
        m.run(start, {0x35});                              // DCR M
        CHECK(m.read(0x3A7C) == 0x3F);
        expected.flags = P;
        CHECK(m.state() == expected);
    }
    SECTION("00 -> FF wraps, leaves CY clear") {
        start.b = 0x00;
        m.run(start, {0x05});                              // DCR B
        expected.b = 0xFF; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("80 -> 7F clears S") {
        start.b = 0x80;
        m.run(start, {0x05});                              // DCR B
        expected.b = 0x7F; expected.flags = 0;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// INX rp (0x03, 0x13, 0x23, 0x33) / DCX rp (0x0B, 0x1B, 0x2B, 0x3B)
// 16-bit increment/decrement. 1 byte. 5 cycles. No flags change, even on
// wraparound to 0000.
// ---------------------------------------------------------------------------

TEST_CASE("INX rp", "[arithmetic][inx]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    start.sp = 0x2400;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("INX B (0x03): 1234 -> 1235") {
        CHECK(m.run(start, {0x03}) == 5);
        expected.c = 0x35;
        CHECK(m.state() == expected);
    }
    SECTION("INX D (0x13): 5678 -> 5679") {
        CHECK(m.run(start, {0x13}) == 5);
        expected.e = 0x79;
        CHECK(m.state() == expected);
    }
    SECTION("INX H (0x23): 2050 -> 2051") {
        CHECK(m.run(start, {0x23}) == 5);
        expected.l = 0x51;
        CHECK(m.state() == expected);
    }
    SECTION("INX SP (0x33): 2400 -> 2401") {
        CHECK(m.run(start, {0x33}) == 5);
        expected.sp = 0x2401;
        CHECK(m.state() == expected);
    }
    SECTION("INX B carries into B: 12FF -> 1300") {
        start.c = 0xFF;
        m.run(start, {0x03});
        expected.b = 0x13; expected.c = 0x00;
        CHECK(m.state() == expected);
    }
    SECTION("INX B wraps FFFF -> 0000 without setting Z") {
        start.b = 0xFF; start.c = 0xFF;
        start.flags = 0;
        expected.flags = 0;
        m.run(start, {0x03});
        expected.b = 0x00; expected.c = 0x00;
        CHECK(m.state() == expected);
    }
    SECTION("INX D example: 38FF -> 3900 (ALP p. 24)") {
        start.d = 0x38; start.e = 0xFF;
        m.run(start, {0x13});
        expected.d = 0x39; expected.e = 0x00;
        CHECK(m.state() == expected);
    }
    SECTION("INX SP example: FFFF -> 0000 (ALP p. 24)") {
        start.sp = 0xFFFF;
        m.run(start, {0x33});
        expected.sp = 0x0000;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("DCX rp", "[arithmetic][dcx]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    start.sp = 0x2400;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("DCX B (0x0B): 1234 -> 1233") {
        CHECK(m.run(start, {0x0B}) == 5);
        expected.c = 0x33;
        CHECK(m.state() == expected);
    }
    SECTION("DCX D (0x1B): 5678 -> 5677") {
        CHECK(m.run(start, {0x1B}) == 5);
        expected.e = 0x77;
        CHECK(m.state() == expected);
    }
    SECTION("DCX H (0x2B): 2050 -> 204F") {
        CHECK(m.run(start, {0x2B}) == 5);
        expected.l = 0x4F;
        CHECK(m.state() == expected);
    }
    SECTION("DCX SP (0x3B): 2400 -> 23FF") {
        CHECK(m.run(start, {0x3B}) == 5);
        expected.sp = 0x23FF;
        CHECK(m.state() == expected);
    }
    SECTION("DCX H example: 9800 -> 97FF (ALP p. 24)") {
        start.h = 0x98; start.l = 0x00;
        m.run(start, {0x2B});
        expected.h = 0x97; expected.l = 0xFF;
        CHECK(m.state() == expected);
    }
    SECTION("DCX B borrows from B: 1200 -> 11FF") {
        start.c = 0x00;
        m.run(start, {0x0B});
        expected.b = 0x11; expected.c = 0xFF;
        CHECK(m.state() == expected);
    }
    SECTION("DCX SP wraps 0000 -> FFFF") {
        start.sp = 0x0000;
        m.run(start, {0x3B});
        expected.sp = 0xFFFF;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// DAD rp (0x09, 0x19, 0x29, 0x39): HL = HL + rp. 1 byte. 10 cycles.
// Only CY changes (set on carry out of bit 15).
// ---------------------------------------------------------------------------

TEST_CASE("DAD rp", "[arithmetic][dad]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);     // CY must be cleared; others kept
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    start.sp = 0x2400;
    CpuState expected = start;
    expected.pc += 1;
    expected.flags = S | Z | AC | P;

    SECTION("DAD B (0x09): 2050 + 1234 = 3284") {
        CHECK(m.run(start, {0x09}) == 10);
        expected.h = 0x32; expected.l = 0x84;
        CHECK(m.state() == expected);
    }
    SECTION("DAD D (0x19): 2050 + 5678 = 76C8") {
        CHECK(m.run(start, {0x19}) == 10);
        expected.h = 0x76; expected.l = 0xC8;
        CHECK(m.state() == expected);
    }
    SECTION("DAD H (0x29): 2050 + 2050 = 40A0") {
        CHECK(m.run(start, {0x29}) == 10);
        expected.h = 0x40; expected.l = 0xA0;
        CHECK(m.state() == expected);
    }
    SECTION("DAD SP (0x39): 2050 + 2400 = 4450") {
        CHECK(m.run(start, {0x39}) == 10);
        expected.h = 0x44; expected.l = 0x50;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("DAD example: A17B + 339F = D51A (ALP p. 24)", "[arithmetic][dad]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.b = 0x33; start.c = 0x9F;
    start.h = 0xA1; start.l = 0x7B;

    m.run(start, {0x09});                                  // DAD B

    CpuState expected = start;
    expected.h = 0xD5; expected.l = 0x1A;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("DAD overflow sets only CY: FFFF + 0001 = 0000", "[arithmetic][dad]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.h = 0xFF; start.l = 0xFF;
    start.d = 0x00; start.e = 0x01;

    m.run(start, {0x19});                                  // DAD D

    CpuState expected = start;
    expected.h = 0x00; expected.l = 0x00;
    expected.flags = CY;                                        // Z is not set
    expected.pc += 1;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// DAA (0x27): decimal adjust A. 1 byte. 4 cycles.
//   1. If low nibble > 9 or AC is set, add 06.
//   2. If high nibble > 9 or CY is set (checked after step 1), add 60 and set CY.
// ---------------------------------------------------------------------------

TEST_CASE("DAA (0x27)", "[arithmetic][daa]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("DAA example: 9B -> 01 with AC and CY (ALP p. 16)") {
        start.a = 0x9B;
        CHECK(m.run(start, {0x27}) == 4);
        expected.a = 0x01; expected.flags = AC | CY;
        CHECK(m.state() == expected);
    }
    SECTION("valid BCD with no flags is unchanged: 45 -> 45") {
        start.a = 0x45;
        m.run(start, {0x27});
        expected.a = 0x45; expected.flags = 0;
        CHECK(m.state() == expected);
    }
    SECTION("AC set adds 06: 42 -> 48") {
        start.a = 0x42; start.flags = AC;
        m.run(start, {0x27});
        expected.a = 0x48; expected.flags = P;
        CHECK(m.state() == expected);
    }
    SECTION("CY set adds 60 and keeps CY: 15 -> 75 (ALP p. 16: CY otherwise unaffected)") {
        start.a = 0x15; start.flags = CY;
        m.run(start, {0x27});
        expected.a = 0x75; expected.flags = CY;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("DAA after ADI: BCD 99 + 01 = 00 with carry", "[arithmetic][daa]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.a = 0x99;
    m.start(start, {0xC6, 0x01, 0x27});                         // ADI 01; DAA

    m.step();                                                   // A = 9A
    m.step();                                                   // DAA

    CpuState expected = start;
    expected.a = 0x00;
    expected.flags = Z | AC | P | CY;
    expected.pc += 3;
    CHECK(m.state() == expected);
}
