// Data transfer group (Intel 8080 manual): MOV, MVI, LXI, LDA, STA, LHLD, SHLD,
// LDAX, STAX, XCHG. None of these instructions change any flags.
//
// Every test starts from CpuState::initial(flag::ALL): every register holds a
// different value and every flag is set, so a test fails if the instruction
// changes the wrong register or clears a flag. Expected values are written in
// terms of the start state (e.g. MOV B,C expects B == start.c).

#include <cstdint>
#include <initializer_list>

#include <catch2/catch_test_macros.hpp>

#include "test_machine.hpp"

using namespace spec;


// ---------------------------------------------------------------------------
// MOV r1,r2 / MOV r,M / MOV M,r (0x40-0x7F, except 0x76 = HLT)
// Register-to-register cycle counts (5) are checked in their own test below, so
// a wrong count doesn't hide whether the data moved correctly.
// ---------------------------------------------------------------------------

TEST_CASE("MOV B,r (0x40-0x47)", "[data_transfer][mov]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("MOV B,B (0x40)") { m.run(start, {0x40});                       CHECK(m.state() == expected); }
    SECTION("MOV B,C (0x41)") { m.run(start, {0x41}); expected.b = start.c; CHECK(m.state() == expected); }
    SECTION("MOV B,D (0x42)") { m.run(start, {0x42}); expected.b = start.d; CHECK(m.state() == expected); }
    SECTION("MOV B,E (0x43)") { m.run(start, {0x43}); expected.b = start.e; CHECK(m.state() == expected); }
    SECTION("MOV B,H (0x44)") { m.run(start, {0x44}); expected.b = start.h; CHECK(m.state() == expected); }
    SECTION("MOV B,L (0x45)") { m.run(start, {0x45}); expected.b = start.l; CHECK(m.state() == expected); }
    SECTION("MOV B,A (0x47)") { m.run(start, {0x47}); expected.b = start.a; CHECK(m.state() == expected); }
    SECTION("MOV B,M (0x46)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x46}) == 7);
        expected.b = 0xA5;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("MOV C,r (0x48-0x4F)", "[data_transfer][mov]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("MOV C,B (0x48)") { m.run(start, {0x48}); expected.c = start.b; CHECK(m.state() == expected); }
    SECTION("MOV C,C (0x49)") { m.run(start, {0x49});                       CHECK(m.state() == expected); }
    SECTION("MOV C,D (0x4A)") { m.run(start, {0x4A}); expected.c = start.d; CHECK(m.state() == expected); }
    SECTION("MOV C,E (0x4B)") { m.run(start, {0x4B}); expected.c = start.e; CHECK(m.state() == expected); }
    SECTION("MOV C,H (0x4C)") { m.run(start, {0x4C}); expected.c = start.h; CHECK(m.state() == expected); }
    SECTION("MOV C,L (0x4D)") { m.run(start, {0x4D}); expected.c = start.l; CHECK(m.state() == expected); }
    SECTION("MOV C,A (0x4F)") { m.run(start, {0x4F}); expected.c = start.a; CHECK(m.state() == expected); }
    SECTION("MOV C,M (0x4E)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x4E}) == 7);
        expected.c = 0xA5;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("MOV D,r (0x50-0x57)", "[data_transfer][mov]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("MOV D,B (0x50)") { m.run(start, {0x50}); expected.d = start.b; CHECK(m.state() == expected); }
    SECTION("MOV D,C (0x51)") { m.run(start, {0x51}); expected.d = start.c; CHECK(m.state() == expected); }
    SECTION("MOV D,D (0x52)") { m.run(start, {0x52});                       CHECK(m.state() == expected); }
    SECTION("MOV D,E (0x53)") { m.run(start, {0x53}); expected.d = start.e; CHECK(m.state() == expected); }
    SECTION("MOV D,H (0x54)") { m.run(start, {0x54}); expected.d = start.h; CHECK(m.state() == expected); }
    SECTION("MOV D,L (0x55)") { m.run(start, {0x55}); expected.d = start.l; CHECK(m.state() == expected); }
    SECTION("MOV D,A (0x57)") { m.run(start, {0x57}); expected.d = start.a; CHECK(m.state() == expected); }
    SECTION("MOV D,M (0x56)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x56}) == 7);
        expected.d = 0xA5;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("MOV E,r (0x58-0x5F)", "[data_transfer][mov]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("MOV E,B (0x58)") { m.run(start, {0x58}); expected.e = start.b; CHECK(m.state() == expected); }
    SECTION("MOV E,C (0x59)") { m.run(start, {0x59}); expected.e = start.c; CHECK(m.state() == expected); }
    SECTION("MOV E,D (0x5A)") { m.run(start, {0x5A}); expected.e = start.d; CHECK(m.state() == expected); }
    SECTION("MOV E,E (0x5B)") { m.run(start, {0x5B});                       CHECK(m.state() == expected); }
    SECTION("MOV E,H (0x5C)") { m.run(start, {0x5C}); expected.e = start.h; CHECK(m.state() == expected); }
    SECTION("MOV E,L (0x5D)") { m.run(start, {0x5D}); expected.e = start.l; CHECK(m.state() == expected); }
    SECTION("MOV E,A (0x5F)") { m.run(start, {0x5F}); expected.e = start.a; CHECK(m.state() == expected); }
    SECTION("MOV E,M (0x5E)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x5E}) == 7);
        expected.e = 0xA5;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("MOV H,r (0x60-0x67)", "[data_transfer][mov]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("MOV H,B (0x60)") { m.run(start, {0x60}); expected.h = start.b; CHECK(m.state() == expected); }
    SECTION("MOV H,C (0x61)") { m.run(start, {0x61}); expected.h = start.c; CHECK(m.state() == expected); }
    SECTION("MOV H,D (0x62)") { m.run(start, {0x62}); expected.h = start.d; CHECK(m.state() == expected); }
    SECTION("MOV H,E (0x63)") { m.run(start, {0x63}); expected.h = start.e; CHECK(m.state() == expected); }
    SECTION("MOV H,H (0x64)") { m.run(start, {0x64});                       CHECK(m.state() == expected); }
    SECTION("MOV H,L (0x65)") { m.run(start, {0x65}); expected.h = start.l; CHECK(m.state() == expected); }
    SECTION("MOV H,A (0x67)") { m.run(start, {0x67}); expected.h = start.a; CHECK(m.state() == expected); }
    SECTION("MOV H,M (0x66)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x66}) == 7);
        expected.h = 0xA5;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("MOV L,r (0x68-0x6F)", "[data_transfer][mov]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("MOV L,B (0x68)") { m.run(start, {0x68}); expected.l = start.b; CHECK(m.state() == expected); }
    SECTION("MOV L,C (0x69)") { m.run(start, {0x69}); expected.l = start.c; CHECK(m.state() == expected); }
    SECTION("MOV L,D (0x6A)") { m.run(start, {0x6A}); expected.l = start.d; CHECK(m.state() == expected); }
    SECTION("MOV L,E (0x6B)") { m.run(start, {0x6B}); expected.l = start.e; CHECK(m.state() == expected); }
    SECTION("MOV L,H (0x6C)") { m.run(start, {0x6C}); expected.l = start.h; CHECK(m.state() == expected); }
    SECTION("MOV L,L (0x6D)") { m.run(start, {0x6D});                       CHECK(m.state() == expected); }
    SECTION("MOV L,A (0x6F)") { m.run(start, {0x6F}); expected.l = start.a; CHECK(m.state() == expected); }
    SECTION("MOV L,M (0x6E)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x6E}) == 7);
        expected.l = 0xA5;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("MOV M,r (0x70-0x77, except 0x76)", "[data_transfer][mov]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 1;

    // Writes to memory at HL; no register changes
    SECTION("MOV M,B (0x70)") { CHECK(m.run(start, {0x70}) == 7); CHECK(m.read(start.hl()) == start.b); CHECK(m.state() == expected); }
    SECTION("MOV M,C (0x71)") { CHECK(m.run(start, {0x71}) == 7); CHECK(m.read(start.hl()) == start.c); CHECK(m.state() == expected); }
    SECTION("MOV M,D (0x72)") { CHECK(m.run(start, {0x72}) == 7); CHECK(m.read(start.hl()) == start.d); CHECK(m.state() == expected); }
    SECTION("MOV M,E (0x73)") { CHECK(m.run(start, {0x73}) == 7); CHECK(m.read(start.hl()) == start.e); CHECK(m.state() == expected); }
    SECTION("MOV M,H (0x74)") { CHECK(m.run(start, {0x74}) == 7); CHECK(m.read(start.hl()) == start.h); CHECK(m.state() == expected); }
    SECTION("MOV M,L (0x75)") { CHECK(m.run(start, {0x75}) == 7); CHECK(m.read(start.hl()) == start.l); CHECK(m.state() == expected); }
    SECTION("MOV M,A (0x77)") { CHECK(m.run(start, {0x77}) == 7); CHECK(m.read(start.hl()) == start.a); CHECK(m.state() == expected); }
}

TEST_CASE("MOV A,r (0x78-0x7F)", "[data_transfer][mov]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 1;

    SECTION("MOV A,B (0x78)") { m.run(start, {0x78}); expected.a = start.b; CHECK(m.state() == expected); }
    SECTION("MOV A,C (0x79)") { m.run(start, {0x79}); expected.a = start.c; CHECK(m.state() == expected); }
    SECTION("MOV A,D (0x7A)") { m.run(start, {0x7A}); expected.a = start.d; CHECK(m.state() == expected); }
    SECTION("MOV A,E (0x7B)") { m.run(start, {0x7B}); expected.a = start.e; CHECK(m.state() == expected); }
    SECTION("MOV A,H (0x7C)") { m.run(start, {0x7C}); expected.a = start.h; CHECK(m.state() == expected); }
    SECTION("MOV A,L (0x7D)") { m.run(start, {0x7D}); expected.a = start.l; CHECK(m.state() == expected); }
    SECTION("MOV A,A (0x7F)") { m.run(start, {0x7F});                       CHECK(m.state() == expected); }
    SECTION("MOV A,M (0x7E)") {
        m.write(start.hl(), 0xA5);
        CHECK(m.run(start, {0x7E}) == 7);
        expected.a = 0xA5;
        CHECK(m.state() == expected);
    }
}

// Manual: MOV r1,r2 is 1 machine cycle, 5 states.
TEST_CASE("MOV r1,r2 takes 5 cycles", "[data_transfer][mov][cycles]") {
    for (int opcode = 0x40; opcode <= 0x7F; opcode++) {
        bool uses_memory = (opcode & 0x07) == 6 || ((opcode >> 3) & 0x07) == 6;
        if (uses_memory)
            continue;    // MOV r,M / MOV M,r / HLT

        INFO("opcode 0x" << std::hex << std::uppercase << opcode);
        TestMachine m;
        CHECK(m.run(CpuState::initial(flag::ALL), {(uint8_t)opcode}) == 5);
    }
}


// ---------------------------------------------------------------------------
// MVI r,data (0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x36, 0x3E)
// 2 bytes. MVI r: 7 cycles. MVI M: 10 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("MVI r,data", "[data_transfer][mvi]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 2;

    SECTION("MVI B,5A (0x06)") { CHECK(m.run(start, {0x06, 0x5A}) == 7); expected.b = 0x5A; CHECK(m.state() == expected); }
    SECTION("MVI C,5A (0x0E)") { CHECK(m.run(start, {0x0E, 0x5A}) == 7); expected.c = 0x5A; CHECK(m.state() == expected); }
    SECTION("MVI D,5A (0x16)") { CHECK(m.run(start, {0x16, 0x5A}) == 7); expected.d = 0x5A; CHECK(m.state() == expected); }
    SECTION("MVI E,5A (0x1E)") { CHECK(m.run(start, {0x1E, 0x5A}) == 7); expected.e = 0x5A; CHECK(m.state() == expected); }
    SECTION("MVI H,5A (0x26)") { CHECK(m.run(start, {0x26, 0x5A}) == 7); expected.h = 0x5A; CHECK(m.state() == expected); }
    SECTION("MVI L,5A (0x2E)") { CHECK(m.run(start, {0x2E, 0x5A}) == 7); expected.l = 0x5A; CHECK(m.state() == expected); }
    SECTION("MVI A,5A (0x3E)") { CHECK(m.run(start, {0x3E, 0x5A}) == 7); expected.a = 0x5A; CHECK(m.state() == expected); }
    SECTION("MVI M,5A (0x36)") {
        CHECK(m.run(start, {0x36, 0x5A}) == 10);
        CHECK(m.read(start.hl()) == 0x5A);
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// LXI rp,data16 (0x01, 0x11, 0x21, 0x31)
// 3 bytes, operand low byte first. 10 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("LXI rp,data16", "[data_transfer][lxi]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    CpuState expected = start;
    expected.pc += 3;

    SECTION("LXI B,ABCD (0x01)") {
        CHECK(m.run(start, {0x01, 0xCD, 0xAB}) == 10);
        expected.b = 0xAB;
        expected.c = 0xCD;
        CHECK(m.state() == expected);
    }
    SECTION("LXI D,ABCD (0x11)") {
        CHECK(m.run(start, {0x11, 0xCD, 0xAB}) == 10);
        expected.d = 0xAB;
        expected.e = 0xCD;
        CHECK(m.state() == expected);
    }
    SECTION("LXI H,ABCD (0x21)") {
        CHECK(m.run(start, {0x21, 0xCD, 0xAB}) == 10);
        expected.h = 0xAB;
        expected.l = 0xCD;
        CHECK(m.state() == expected);
    }
    SECTION("LXI SP,ABCD (0x31)") {
        CHECK(m.run(start, {0x31, 0xCD, 0xAB}) == 10);
        expected.sp = 0xABCD;
        CHECK(m.state() == expected);
    }
}


// ---------------------------------------------------------------------------
// LDA addr (0x3A) / STA addr (0x32)
// 3 bytes, address low byte first. 13 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("LDA addr (0x3A)", "[data_transfer][lda]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    m.write(0x3456, 0x5A);

    CHECK(m.run(start, {0x3A, 0x56, 0x34}) == 13);    // LDA 3456

    CpuState expected = start;
    expected.a = 0x5A;
    expected.pc += 3;
    CHECK(m.state() == expected);
}

TEST_CASE("STA addr (0x32)", "[data_transfer][sta]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);

    CHECK(m.run(start, {0x32, 0x56, 0x34}) == 13);    // STA 3456

    CHECK(m.read(0x3456) == start.a);
    CHECK(m.read(0x3455) == 0x00);    // neighbors untouched
    CHECK(m.read(0x3457) == 0x00);
    CpuState expected = start;
    expected.pc += 3;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// LHLD addr (0x2A) / SHLD addr (0x22)
// L <-> (addr), H <-> (addr + 1). 3 bytes. 16 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("LHLD addr (0x2A)", "[data_transfer][lhld]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    m.write(0x3456, 0xCD);
    m.write(0x3457, 0xAB);

    CHECK(m.run(start, {0x2A, 0x56, 0x34}) == 16);    // LHLD 3456

    CpuState expected = start;
    expected.l = 0xCD;
    expected.h = 0xAB;
    expected.pc += 3;
    CHECK(m.state() == expected);
}

TEST_CASE("SHLD addr (0x22)", "[data_transfer][shld]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);

    CHECK(m.run(start, {0x22, 0x56, 0x34}) == 16);    // SHLD 3456

    CHECK(m.read(0x3456) == start.l);
    CHECK(m.read(0x3457) == start.h);
    CpuState expected = start;
    expected.pc += 3;
    CHECK(m.state() == expected);
}

TEST_CASE("LHLD at FFFF reads H from 0000", "[data_transfer][lhld]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    m.write(0xFFFF, 0xCD);
    m.write(0x0000, 0xAB);

    CHECK(m.run(start, {0x2A, 0xFF, 0xFF}) == 16);     // LHLD FFFF

    CpuState expected = start;
    expected.l = 0xCD;
    expected.h = 0xAB;
    expected.pc += 3;
    CHECK(m.state() == expected);
}

TEST_CASE("SHLD at FFFF writes H to 0000", "[data_transfer][shld]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);

    CHECK(m.run(start, {0x22, 0xFF, 0xFF}) == 16);     // SHLD FFFF

    CHECK(m.read(0xFFFF) == start.l);
    CHECK(m.read(0x0000) == start.h);
    CpuState expected = start;
    expected.pc += 3;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// LDAX rp (0x0A, 0x1A) / STAX rp (0x02, 0x12)
// A <-> memory at BC or DE. 1 byte. 7 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("LDAX B (0x0A)", "[data_transfer][ldax]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    m.write(start.bc(), 0x5A);

    CHECK(m.run(start, {0x0A}) == 7);

    CpuState expected = start;
    expected.a = 0x5A;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("LDAX D (0x1A)", "[data_transfer][ldax]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);
    m.write(start.de(), 0x5A);

    CHECK(m.run(start, {0x1A}) == 7);

    CpuState expected = start;
    expected.a = 0x5A;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("STAX B (0x02)", "[data_transfer][stax]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);

    CHECK(m.run(start, {0x02}) == 7);

    CHECK(m.read(start.bc()) == start.a);
    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("STAX D (0x12)", "[data_transfer][stax]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);

    CHECK(m.run(start, {0x12}) == 7);

    CHECK(m.read(start.de()) == start.a);
    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// XCHG (0xEB): swap HL and DE. 1 byte. 4 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("XCHG (0xEB)", "[data_transfer][xchg]") {
    TestMachine m;
    CpuState start = CpuState::initial(flag::ALL);

    CHECK(m.run(start, {0xEB}) == 4);

    CpuState expected = start;
    expected.d = start.h;
    expected.e = start.l;
    expected.h = start.d;
    expected.l = start.e;
    expected.pc += 1;
    CHECK(m.state() == expected);
}
