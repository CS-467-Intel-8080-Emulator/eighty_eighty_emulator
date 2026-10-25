// Logical group (Intel 8080 manual): ANA, ANI, XRA, XRI, ORA, ORI, CMP, CPI,
// RLC, RRC, RAL, RAR, CMA, CMC, STC.
//
// Flag rules:
//   ANA/ANI: S, Z, P from the result; CY cleared. AC is set to bit 3 of
//            (A OR operand). The 1975 manuals don't say this (the User's
//            Manual says ANA "affects" AC and ANI clears it; the 1975 ALP
//            doesn't list AC for either), but Intel's later 8080/8085 Assembly
//            Language Programming Manual (p. 1-12) states it.
//   XRA/XRI, ORA/ORI: S, Z, P from the result; CY and AC cleared.
//   CMP/CPI: flags exactly as SUB (A - operand), but A is not changed.
//            Z = equal, CY = A < operand (unsigned).
//   RLC/RRC/RAL/RAR: only CY changes.
//   CMA: no flags. CMC: CY inverted, nothing else. STC: CY set, nothing else.
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

#include <catch2/catch_test_macros.hpp>

#include "test_machine.hpp"

using namespace spec;
using namespace spec::flag;

// ---------------------------------------------------------------------------
// ANA r / ANA M (0xA0-0xA7): A = A AND r. 4 cycles (M: 7).
// Starts with all flags set, so CY must be cleared.
// ---------------------------------------------------------------------------

TEST_CASE("ANA r (0xA0-0xA7)", "[logical][ana]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("ANA B (0xA0): 9C AND 12 = 10") {
        CHECK(m.run(start, {0xA0}) == 4);
        expected.a = 0x10; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("ANA C (0xA1): 9C AND 34 = 14") {
        CHECK(m.run(start, {0xA1}) == 4);
        expected.a = 0x14; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("ANA D (0xA2): 9C AND 56 = 14") {
        CHECK(m.run(start, {0xA2}) == 4);
        expected.a = 0x14; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("ANA E (0xA3): 9C AND 78 = 18") {
        CHECK(m.run(start, {0xA3}) == 4);
        expected.a = 0x18; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("ANA H (0xA4): 9C AND 20 = 00") {
        CHECK(m.run(start, {0xA4}) == 4);
        expected.a = 0x00; expected.flags = Z | AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("ANA L (0xA5): 9C AND 50 = 10") {
        CHECK(m.run(start, {0xA5}) == 4);
        expected.a = 0x10; expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("ANA M (0xA6): 9C AND 0F = 0C") {
        m.write(start.hl(),0x0F);
        CHECK(m.run(start, {0xA6}) == 7);
        expected.a = 0x0C; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("ANA A (0xA7): 9C AND 9C = 9C") {
        CHECK(m.run(start, {0xA7}) == 4);
        expected.a = 0x9C; expected.flags = S | AC | P;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("ANA example: FC AND 0F = 0C (ALP p. 19; flags not stated)", "[logical][ana]") {
    TestMachine m;
    CpuState start = CpuState::initial(CY);
    start.a = 0xFC;
    start.c = 0x0F;

    m.run(start, {0xA1});                                       // ANA C

    CpuState expected = start;
    expected.a = 0x0C;
    expected.flags = AC | P;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("ANA with bit 3 clear in both operands clears AC: F0 AND 37 = 30", "[logical][ana]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0xF0;
    start.b = 0x37;

    m.run(start, {0xA0});                                       // ANA B

    CpuState expected = start;
    expected.a = 0x30;
    expected.flags = P;
    expected.pc += 1;
    CHECK(m.state() == expected);
}


// ALP p. 20, "testing for change of status": STAT1 = 5C is in B, STAT2 = 78 in
// A. XRA B marks the bits that changed; ANA B then keeps those that were set in
// STAT1. The manual loads A and B from memory first; here they are set directly.
TEST_CASE("XRA/ANA example: status change 5C -> 78 (ALP p. 20; flags not stated)", "[logical][xra][ana]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x78;
    start.b = 0x5C;
    m.start(start, {0xA8, 0xA0});                               // XRA B; ANA B

    m.step();
    CpuState expected = start;
    expected.a = 0x24;                                          // bits 2 and 5 changed
    expected.flags = P;
    expected.pc += 1;
    CHECK(m.state() == expected);

    m.step();
    expected.a = 0x04;                                          // bit 2 was set in STAT1
    expected.flags = AC;                                        // AC: bit 3 of 24 OR 5C
    expected.pc += 1;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// XRA r / XRA M (0xA8-0xAF): A = A XOR r. 4 cycles (M: 7).
// Starts with all flags set, so CY and AC must be cleared.
// ---------------------------------------------------------------------------

TEST_CASE("XRA r (0xA8-0xAF)", "[logical][xra]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("XRA B (0xA8): 9C XOR 12 = 8E") {
        CHECK(m.run(start, {0xA8}) == 4);
        expected.a = 0x8E; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("XRA C (0xA9): 9C XOR 34 = A8") {
        CHECK(m.run(start, {0xA9}) == 4);
        expected.a = 0xA8; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("XRA D (0xAA): 9C XOR 56 = CA") {
        CHECK(m.run(start, {0xAA}) == 4);
        expected.a = 0xCA; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("XRA E (0xAB): 9C XOR 78 = E4") {
        CHECK(m.run(start, {0xAB}) == 4);
        expected.a = 0xE4; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("XRA H (0xAC): 9C XOR 20 = BC") {
        CHECK(m.run(start, {0xAC}) == 4);
        expected.a = 0xBC; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("XRA L (0xAD): 9C XOR 50 = CC") {
        CHECK(m.run(start, {0xAD}) == 4);
        expected.a = 0xCC; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("XRA M (0xAE): 9C XOR 9C = 00") {
        m.write(start.hl(),0x9C);
        CHECK(m.run(start, {0xAE}) == 7);
        expected.a = 0x00; expected.flags = Z | P;
        CHECK(m.state() == expected);
    }
    SECTION("XRA A (0xAF): clears A") {
        CHECK(m.run(start, {0xAF}) == 4);
        expected.a = 0x00; expected.flags = Z | P;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// ORA r / ORA M (0xB0-0xB7): A = A OR r. 4 cycles (M: 7).
// Starts with all flags set, so CY and AC must be cleared.
// ---------------------------------------------------------------------------

TEST_CASE("ORA r (0xB0-0xB7)", "[logical][ora]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("ORA B (0xB0): 9C OR 12 = 9E") {
        CHECK(m.run(start, {0xB0}) == 4);
        expected.a = 0x9E; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("ORA C (0xB1): 9C OR 34 = BC") {
        CHECK(m.run(start, {0xB1}) == 4);
        expected.a = 0xBC; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("ORA D (0xB2): 9C OR 56 = DE") {
        CHECK(m.run(start, {0xB2}) == 4);
        expected.a = 0xDE; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("ORA E (0xB3): 9C OR 78 = FC") {
        CHECK(m.run(start, {0xB3}) == 4);
        expected.a = 0xFC; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("ORA H (0xB4): 9C OR 20 = BC") {
        CHECK(m.run(start, {0xB4}) == 4);
        expected.a = 0xBC; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("ORA L (0xB5): 9C OR 50 = DC") {
        CHECK(m.run(start, {0xB5}) == 4);
        expected.a = 0xDC; expected.flags = S;
        CHECK(m.state() == expected);
    }
    SECTION("ORA M (0xB6): 9C OR 63 = FF") {
        m.write(start.hl(),0x63);
        CHECK(m.run(start, {0xB6}) == 7);
        expected.a = 0xFF; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
    SECTION("ORA A (0xB7): 9C OR 9C = 9C") {
        CHECK(m.run(start, {0xB7}) == 4);
        expected.a = 0x9C; expected.flags = S | P;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("ORA example: 33 OR 0F = 3F (ALP p. 20; flags not stated)", "[logical][ora]") {
    TestMachine m;
    CpuState start = CpuState::initial(CY | AC);
    start.a = 0x33;
    start.c = 0x0F;

    m.run(start, {0xB1});                                       // ORA C

    CpuState expected = start;
    expected.a = 0x3F;
    expected.flags = P;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("ORA of zero with zero sets Z: 00 OR 00 = 00", "[logical][ora]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.a = 0x00;
    start.b = 0x00;

    m.run(start, {0xB0});                                       // ORA B

    CpuState expected = start;
    expected.flags = Z | P;
    expected.pc += 1;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// CMP r / CMP M (0xB8-0xBF): flags of A - r; A unchanged. 4 cycles (M: 7).
// Starts with all flags set.
// ---------------------------------------------------------------------------

TEST_CASE("CMP r (0xB8-0xBF)", "[logical][cmp]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    start.b = 0x12; start.c = 0x34;
    start.d = 0x56; start.e = 0x78;
    start.h = 0x20; start.l = 0x50;
    CpuState expected = start;
    expected.pc += 1;

    SECTION("CMP B (0xB8): 9C vs 12") {
        CHECK(m.run(start, {0xB8}) == 4);
        expected.flags = S | AC;
        CHECK(m.state() == expected);
    }
    SECTION("CMP C (0xB9): 9C vs 34") {
        CHECK(m.run(start, {0xB9}) == 4);
        expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("CMP D (0xBA): 9C vs 56") {
        CHECK(m.run(start, {0xBA}) == 4);
        expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("CMP E (0xBB): 9C vs 78") {
        CHECK(m.run(start, {0xBB}) == 4);
        expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("CMP H (0xBC): 9C vs 20") {
        CHECK(m.run(start, {0xBC}) == 4);
        expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("CMP L (0xBD): 9C vs 50") {
        CHECK(m.run(start, {0xBD}) == 4);
        expected.flags = AC;
        CHECK(m.state() == expected);
    }
    SECTION("CMP M (0xBE): 9C vs A5 (A smaller: CY)") {
        m.write(start.hl(),0xA5);
        CHECK(m.run(start, {0xBE}) == 7);
        expected.flags = S | AC | CY;
        CHECK(m.state() == expected);
    }
    SECTION("CMP A (0xBF): equal sets Z") {
        CHECK(m.run(start, {0xBF}) == 4);
        expected.flags = Z | AC | P;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("CMP: greater/less/mixed-sign/equal", "[logical][cmp]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("CMP example 1: A=0A > E=05, CY and Z clear (ALP p. 20)") {
        start.a = 0x0A; start.e = 0x05;
        m.run(start, {0xBB});                                   // CMP E
        expected.a = 0x0A; expected.e = 0x05; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("CMP example 2: A=02 < E=05, CY set and Z clear (ALP p. 21)") {
        start.a = 0x02; start.e = 0x05;
        m.run(start, {0xBB});                                   // CMP E
        expected.a = 0x02; expected.e = 0x05; expected.flags = S | CY;
        CHECK(m.state() == expected);
    }
    SECTION("CMP example 3: A=E5 vs E=05, CY clear, A is larger unsigned (ALP p. 21)") {
        start.a = 0xE5; start.e = 0x05;
        m.run(start, {0xBB});                                   // CMP E
        expected.a = 0xE5; expected.e = 0x05; expected.flags = S | AC;
        CHECK(m.state() == expected);
    }
    SECTION("A=05 = E=05: Z set (not a manual example)") {
        start.a = 0x05; start.e = 0x05;
        m.run(start, {0xBB});                                   // CMP E
        expected.a = 0x05; expected.e = 0x05; expected.flags = Z | AC | P;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// Immediates: ANI (0xE6), XRI (0xEE), ORI (0xF6), CPI (0xFE)
// 2 bytes. 7 cycles. Same flag rules as the register forms.
// ---------------------------------------------------------------------------

TEST_CASE("ANI data (0xE6)", "[logical][ani]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    CpuState expected = start;
    expected.pc += 2;

    SECTION("9C AND 0F = 0C") {
        CHECK(m.run(start, {0xE6, 0x0F}) == 7);
        expected.a = 0x0C; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
    // The manual first copies 3A from C with MOV A,C; here A is set directly.
    SECTION("ANI example: 3A AND 0F = 0A (ALP p. 28; flags not stated)") {
        start.a = 0x3A;
        CHECK(m.run(start, {0xE6, 0x0F}) == 7);
        expected.a = 0x0A; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("XRI data (0xEE)", "[logical][xri]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;

    CHECK(m.run(start, {0xEE, 0xFF}) == 7);                     // XRI FF: inverts A

    CpuState expected = start;
    expected.a = 0x63;
    expected.flags = P;
    expected.pc += 2;
    CHECK(m.state() == expected);
}

TEST_CASE("XRI example: 3B XOR 81 = BA (ALP p. 29; flags not stated)", "[logical][xri]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x3B;

    m.run(start, {0xEE, 0x81});                                 // XRI 81

    CpuState expected = start;
    expected.a = 0xBA;
    expected.flags = S;
    expected.pc += 2;
    CHECK(m.state() == expected);
}

TEST_CASE("ORI data (0xF6)", "[logical][ori]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;

    CHECK(m.run(start, {0xF6, 0x0F}) == 7);                     // ORI 0F

    CpuState expected = start;
    expected.a = 0x9F;
    expected.flags = S | P;
    expected.pc += 2;
    CHECK(m.state() == expected);
}

// The manual first copies B5 from C with MOV A,C; here A is set directly.
TEST_CASE("ORI example: B5 OR 0F = BF (ALP p. 29; flags not stated)", "[logical][ori]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0xB5;

    m.run(start, {0xF6, 0x0F});                                 // ORI 0F

    CpuState expected = start;
    expected.a = 0xBF;
    expected.flags = S;
    expected.pc += 2;
    CHECK(m.state() == expected);
}

TEST_CASE("CPI data (0xFE)", "[logical][cpi]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    start.a = 0x9C;
    CpuState expected = start;
    expected.pc += 2;

    SECTION("equal: 9C vs 9C sets Z") {
        CHECK(m.run(start, {0xFE, 0x9C}) == 7);
        expected.flags = Z | AC | P;
        CHECK(m.state() == expected);
    }
    SECTION("A smaller: 9C vs A5 sets CY") {
        CHECK(m.run(start, {0xFE, 0xA5}) == 7);
        expected.flags = S | AC | CY;
        CHECK(m.state() == expected);
    }
    SECTION("CPI example: 4A vs 40, A larger: Z and CY clear (ALP p. 29)") {
        start.a = 0x4A;
        CHECK(m.run(start, {0xFE, 0x40}) == 7);
        expected.a = 0x4A; expected.flags = AC | P;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// Rotates: RLC (0x07), RRC (0x0F), RAL (0x17), RAR (0x1F). 1 byte. 4 cycles.
//   RLC: bit 7 -> CY and bit 0       RRC: bit 0 -> CY and bit 7
//   RAL: bit 7 -> CY, old CY -> bit 0 RAR: bit 0 -> CY, old CY -> bit 7
// Only CY changes; each test sets the other flags to show they're kept.
// ---------------------------------------------------------------------------

TEST_CASE("RLC (0x07)", "[logical][rotate][rlc]") {
    TestMachine m;
    CpuState expected;

    SECTION("RLC example: F2 -> E5, CY set (ALP p. 21)") {
        CpuState start = CpuState::initial(0);
        start.a = 0xF2;
        CHECK(m.run(start, {0x07}) == 4);
        expected = start; expected.a = 0xE5; expected.flags = CY; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("9C -> 39, CY set, other flags kept") {
        CpuState start = CpuState::initial(S | Z | AC | P);
        start.a = 0x9C;
        CHECK(m.run(start, {0x07}) == 4);
        expected = start; expected.a = 0x39; expected.flags = ALL; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("40 -> 80, CY cleared, other flags kept") {
        CpuState start = CpuState::initial(ALL);
        start.a = 0x40;
        CHECK(m.run(start, {0x07}) == 4);
        expected = start; expected.a = 0x80; expected.flags = S | Z | AC | P; expected.pc += 1;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("RRC (0x0F)", "[logical][rotate][rrc]") {
    TestMachine m;
    CpuState expected;

    SECTION("RRC example: F2 -> 79, CY clear (ALP p. 21)") {
        CpuState start = CpuState::initial(CY);
        start.a = 0xF2;
        CHECK(m.run(start, {0x0F}) == 4);
        expected = start; expected.a = 0x79; expected.flags = 0; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("9C -> 4E, CY cleared, other flags kept") {
        CpuState start = CpuState::initial(ALL);
        start.a = 0x9C;
        CHECK(m.run(start, {0x0F}) == 4);
        expected = start; expected.a = 0x4E; expected.flags = S | Z | AC | P; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("01 -> 80, CY set, other flags kept") {
        CpuState start = CpuState::initial(S | Z | AC | P);
        start.a = 0x01;
        CHECK(m.run(start, {0x0F}) == 4);
        expected = start; expected.a = 0x80; expected.flags = ALL; expected.pc += 1;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("RAL (0x17)", "[logical][rotate][ral]") {
    TestMachine m;
    CpuState expected;

    SECTION("RAL example: B5 with CY clear -> 6A, CY set (ALP p. 22)") {
        CpuState start = CpuState::initial(0);
        start.a = 0xB5;
        CHECK(m.run(start, {0x17}) == 4);
        expected = start; expected.a = 0x6A; expected.flags = CY; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("5C with CY set -> B9 (old CY into bit 0), CY cleared") {
        CpuState start = CpuState::initial(ALL);
        start.a = 0x5C;
        CHECK(m.run(start, {0x17}) == 4);
        expected = start; expected.a = 0xB9; expected.flags = S | Z | AC | P; expected.pc += 1;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("RAR (0x1F)", "[logical][rotate][rar]") {
    TestMachine m;
    CpuState expected;

    SECTION("RAR example: 6A with CY set -> B5 (old CY into bit 7), CY clear (ALP p. 22)") {
        CpuState start = CpuState::initial(CY);
        start.a = 0x6A;
        CHECK(m.run(start, {0x1F}) == 4);
        expected = start; expected.a = 0xB5; expected.flags = 0; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("01 with CY clear -> 00, CY set, Z not set") {
        CpuState start = CpuState::initial(0);
        start.a = 0x01;
        CHECK(m.run(start, {0x1F}) == 4);
        expected = start; expected.a = 0x00; expected.flags = CY; expected.pc += 1;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// CMA (0x2F), CMC (0x3F), STC (0x37). 1 byte. 4 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("CMA (0x2F): A = NOT A, no flags change", "[logical][cma]") {
    TestMachine m;

    SECTION("CMA example: 51 -> AE, flags all clear stay clear (ALP p. 15)") {
        CpuState start = CpuState::initial(0);
        start.a = 0x51;
        CHECK(m.run(start, {0x2F}) == 4);
        CpuState expected = start;
        expected.a = 0xAE; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("9C -> 63, flags all set stay set") {
        CpuState start = CpuState::initial(ALL);
        start.a = 0x9C;
        CHECK(m.run(start, {0x2F}) == 4);
        CpuState expected = start;
        expected.a = 0x63; expected.pc += 1;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("CMC (0x3F): CY inverted, nothing else changes", "[logical][cmc]") {
    TestMachine m;

    SECTION("CY set -> clear") {
        CpuState start = CpuState::initial(ALL);
        CHECK(m.run(start, {0x3F}) == 4);
        CpuState expected = start;
        expected.flags = S | Z | AC | P; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("CY clear -> set") {
        CpuState start = CpuState::initial(0);
        CHECK(m.run(start, {0x3F}) == 4);
        CpuState expected = start;
        expected.flags = CY; expected.pc += 1;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("STC (0x37): CY set, nothing else changes", "[logical][stc]") {
    TestMachine m;

    SECTION("CY clear -> set, other flags kept") {
        CpuState start = CpuState::initial(S | Z | AC | P);
        CHECK(m.run(start, {0x37}) == 4);
        CpuState expected = start;
        expected.flags = ALL; expected.pc += 1;
        CHECK(m.state() == expected);
    }
    SECTION("CY clear -> set, from no flags") {
        CpuState start = CpuState::initial(0);
        CHECK(m.run(start, {0x37}) == 4);
        CpuState expected = start;
        expected.flags = CY; expected.pc += 1;
        CHECK(m.state() == expected);
    }
}
