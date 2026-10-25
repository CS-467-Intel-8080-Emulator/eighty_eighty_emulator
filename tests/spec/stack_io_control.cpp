// Stack, I/O and machine control groups (Intel 8080 manual):
//   PUSH, POP, XTHL, SPHL, IN, OUT, EI, DI, HLT, NOP, and hardware interrupts.
// None of these change flags, except POP PSW, which loads them.
//
// Stack layout: a push stores the high byte at SP-1 and the low byte at SP-2,
// then SP -= 2. A pop reads the low byte at SP and the high byte at SP+1,
// then SP += 2.
//
// PSW is A plus the flag byte. On the stack the flag byte is S Z 0 AC 0 P 1 CY
// (bit 1 always 1, bits 3 and 5 always 0); A is the high byte.
//
// Expected values are written in terms of the start state (start.b,
// start.sp, start.pc, ...), never the specific values CpuState::initial() uses.

#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "test_machine.hpp"

using namespace spec;
using namespace spec::flag;

// ---------------------------------------------------------------------------
// PUSH rp: 1 byte, 11 cycles. PUSH B C5, PUSH D D5, PUSH H E5, PUSH PSW F5
// ---------------------------------------------------------------------------

TEST_CASE("PUSH B (0xC5)", "[stack][push]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xC5}) == 11);

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
    CHECK(m.read(start.sp - 1) == start.b);
    CHECK(m.read(start.sp - 2) == start.c);
}

TEST_CASE("PUSH D (0xD5)", "[stack][push]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xD5}) == 11);

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
    CHECK(m.read(start.sp - 1) == start.d);
    CHECK(m.read(start.sp - 2) == start.e);
}

TEST_CASE("PUSH H (0xE5)", "[stack][push]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xE5}) == 11);

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
    CHECK(m.read(start.sp - 1) == start.h);
    CHECK(m.read(start.sp - 2) == start.l);
}

TEST_CASE("PUSH PSW (0xF5)", "[stack][push]") {
    TestMachine m;

    SECTION("all flags set: flag byte D7") {
        CpuState start = CpuState::initial(ALL);
        CHECK(m.run(start, {0xF5}) == 11);
        CpuState expected = start;
        expected.pc += 1;
        expected.sp = start.sp - 2;
        CHECK(m.state() == expected);
        CHECK(m.read(start.sp - 1) == start.a);
        CHECK(m.read(start.sp - 2) == 0xD7);    // S Z 0 AC 0 P 1 CY = 1101 0111
    }
    SECTION("all flags clear: flag byte 02") {
        CpuState start = CpuState::initial(0);
        CHECK(m.run(start, {0xF5}) == 11);
        CHECK(m.read(start.sp - 1) == start.a);
        CHECK(m.read(start.sp - 2) == 0x02);    // only the always-1 bit
    }
    SECTION("S and P set: flag byte 86") {
        CpuState start = CpuState::initial(S | P);
        m.run(start, {0xF5});
        CHECK(m.read(start.sp - 2) == 0x86);    // 1000 0110
    }
    SECTION("Z, AC and CY set: flag byte 53") {
        CpuState start = CpuState::initial(Z | AC | CY);
        m.run(start, {0xF5});
        CHECK(m.read(start.sp - 2) == 0x53);    // 0101 0011
    }
    SECTION("PUSH PSW example: A=1F, CY Z P set -> 1F at 5029, 47 at 5028 (ALP p. 23)") {
        CpuState start = CpuState::initial(Z | P | CY);
        start.a = 0x1F;
        start.sp = 0x502A;
        CHECK(m.run(start, {0xF5}) == 11);
        CpuState expected = start;
        expected.pc += 1;
        expected.sp = 0x5028;
        CHECK(m.state() == expected);
        CHECK(m.read(0x5029) == 0x1F);          // A
        CHECK(m.read(0x5028) == 0x47);          // 0100 0111
    }
}

TEST_CASE("PUSH with SP = 0001 wraps around 0000", "[stack][push]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    start.sp = 0x0001;

    m.run(start, {0xC5});                 // PUSH B

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = 0xFFFF;
    CHECK(m.state() == expected);
    CHECK(m.read(0x0000) == start.b);
    CHECK(m.read(0xFFFF) == start.c);
}


// ---------------------------------------------------------------------------
// POP rp: 1 byte, 10 cycles. POP B C1, POP D D1, POP H E1, POP PSW F1
// The stack holds ABCD at SP (CD at SP, AB at SP+1).
// ---------------------------------------------------------------------------

TEST_CASE("POP B (0xC1)", "[stack][pop]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    m.write(start.sp, 0xCD);              // low byte
    m.write(start.sp + 1, 0xAB);          // high byte

    CHECK(m.run(start, {0xC1}) == 10);

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = start.sp + 2;
    expected.b = 0xAB;
    expected.c = 0xCD;
    CHECK(m.state() == expected);
}

TEST_CASE("POP D (0xD1)", "[stack][pop]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    m.write(start.sp, 0xCD);              // low byte
    m.write(start.sp + 1, 0xAB);          // high byte

    CHECK(m.run(start, {0xD1}) == 10);

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = start.sp + 2;
    expected.d = 0xAB;
    expected.e = 0xCD;
    CHECK(m.state() == expected);
}

TEST_CASE("POP H (0xE1)", "[stack][pop]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    m.write(start.sp, 0xCD);              // low byte
    m.write(start.sp + 1, 0xAB);          // high byte

    CHECK(m.run(start, {0xE1}) == 10);

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = start.sp + 2;
    expected.h = 0xAB;
    expected.l = 0xCD;
    CHECK(m.state() == expected);
}

TEST_CASE("POP PSW (0xF1)", "[stack][pop]") {
    TestMachine m;

    SECTION("flag byte FF sets every flag") {
        CpuState start = CpuState::initial(0);
        m.write(start.sp, 0xFF);          // flag byte
        m.write(start.sp + 1, 0x3E);      // A
        CHECK(m.run(start, {0xF1}) == 10);
        CpuState expected = start;
        expected.pc += 1;
        expected.sp = start.sp + 2;
        expected.a = 0x3E;
        expected.flags = ALL;
        CHECK(m.state() == expected);
    }
    SECTION("flag byte 00 clears every flag") {
        CpuState start = CpuState::initial(ALL);
        m.write(start.sp, 0x00);
        m.write(start.sp + 1, 0x3E);
        CHECK(m.run(start, {0xF1}) == 10);
        CpuState expected = start;
        expected.pc += 1;
        expected.sp = start.sp + 2;
        expected.a = 0x3E;
        expected.flags = 0;
        CHECK(m.state() == expected);
    }
    SECTION("flag byte 85 sets S, P, CY only") {
        CpuState start = CpuState::initial(Z | AC);
        m.write(start.sp, 0x85);          // 1000 0101
        m.write(start.sp + 1, 0x3E);
        m.run(start, {0xF1});
        CHECK((m.state().flags & ALL) == (S | P | CY));
    }
    SECTION("flag byte 50 sets Z, AC only") {
        CpuState start = CpuState::initial(S | P | CY);
        m.write(start.sp, 0x50);          // 0101 0000
        m.write(start.sp + 1, 0x3E);
        m.run(start, {0xF1});
        CHECK((m.state().flags & ALL) == (Z | AC));
    }
    SECTION("POP PSW example: C3 FF at 2C00 -> A=FF, S Z CY set, AC P clear (ALP p. 23)") {
        CpuState start = CpuState::initial(AC | P);
        start.sp = 0x2C00;
        m.write(0x2C00, 0xC3);            // flag byte: 1100 0011
        m.write(0x2C01, 0xFF);            // A
        CHECK(m.run(start, {0xF1}) == 10);
        CpuState expected = start;
        expected.pc += 1;
        expected.sp = 0x2C02;
        expected.a = 0xFF;
        expected.flags = S | Z | CY;
        CHECK(m.state() == expected);
    }
}

TEST_CASE("POP with SP = FFFF wraps around 0000", "[stack][pop]") {
    TestMachine m;
    m.write(0xFFFF, 0xCD);                // low byte
    m.write(0x0000, 0xAB);                // high byte
    CpuState start = CpuState::initial(0);
    start.sp = 0xFFFF;

    m.run(start, {0xC1});                 // POP B

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = 0x0001;
    expected.b = 0xAB;
    expected.c = 0xCD;
    CHECK(m.state() == expected);
}

TEST_CASE("PUSH PSW then POP PSW restores A and flags", "[stack][push][pop]") {
    TestMachine m;
    CpuState start = CpuState::initial(S | AC | CY);
    // PUSH PSW; MVI A,00; XRA A (A=0, flags change); POP PSW
    m.start(start, {0xF5, 0x3E, 0x00, 0xAF, 0xF1});

    m.step();
    m.step();
    m.step();
    m.step();

    CpuState expected = start;
    expected.pc += 5;
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// XTHL (0xE3): swap L with (SP) and H with (SP+1). 1 byte, 18 cycles.
// SPHL (0xF9): SP = HL. 1 byte, 5 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("XTHL (0xE3)", "[stack][xthl]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    m.write(start.sp, 0xCD);              // low byte
    m.write(start.sp + 1, 0xAB);          // high byte

    CHECK(m.run(start, {0xE3}) == 18);

    CpuState expected = start;
    expected.pc += 1;
    expected.h = 0xAB;
    expected.l = 0xCD;
    CHECK(m.state() == expected);         // SP unchanged
    CHECK(m.read(start.sp) == start.l);
    CHECK(m.read(start.sp + 1) == start.h);
}

TEST_CASE("SPHL (0xF9)", "[stack][sphl]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xF9}) == 5);

    CpuState expected = start;
    expected.pc += 1;
    expected.sp = start.hl();
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// IN port (0xDB): A = value from the port. 2 bytes, 10 cycles.
// OUT port (0xD3): send A to the port. 2 bytes, 10 cycles.
// ---------------------------------------------------------------------------

TEST_CASE("IN port (0xDB)", "[io][in]") {
    TestMachine m;
    m.io().inputs[0x42] = 0x5A;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xDB, 0x42}) == 10);                    // IN 42

    CpuState expected = start;
    expected.pc += 2;
    expected.a = 0x5A;
    CHECK(m.state() == expected);
    REQUIRE(m.io().reads.size() == 1);
    CHECK(m.io().reads[0] == 0x42);
    CHECK(m.io().writes.empty());
}

TEST_CASE("OUT port (0xD3)", "[io][out]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xD3, 0x42}) == 10);                    // OUT 42

    CpuState expected = start;
    expected.pc += 2;
    CHECK(m.state() == expected);
    REQUIRE(m.io().writes.size() == 1);
    CHECK(m.io().writes[0].first == 0x42);     // port
    CHECK(m.io().writes[0].second == start.a);
    CHECK(m.io().reads.empty());
}

TEST_CASE("IN and OUT reach ports 00 and FF", "[io]") {
    TestMachine m;
    m.io().inputs[0x00] = 0x11;
    m.io().inputs[0xFF] = 0xEE;
    CpuState start = CpuState::initial(0);
    // IN 00; OUT FF; IN FF; OUT 00
    m.start(start, {0xDB, 0x00, 0xD3, 0xFF, 0xDB, 0xFF, 0xD3, 0x00});

    m.step();
    m.step();
    m.step();
    m.step();

    REQUIRE(m.io().reads.size() == 2);
    CHECK(m.io().reads[0] == 0x00);
    CHECK(m.io().reads[1] == 0xFF);
    REQUIRE(m.io().writes.size() == 2);
    CHECK(m.io().writes[0].first == 0xFF);
    CHECK(m.io().writes[0].second == 0x11);
    CHECK(m.io().writes[1].first == 0x00);
    CHECK(m.io().writes[1].second == 0xEE);
}


// ---------------------------------------------------------------------------
// NOP (0x00), EI (0xFB), DI (0xF3): 1 byte, 4 cycles, no visible register change
// HLT (0x76): 1 byte, 7 cycles; the CPU stops until an interrupt
// ---------------------------------------------------------------------------

TEST_CASE("NOP (0x00)", "[control][nop]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0x00}) == 4);

    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("EI (0xFB)", "[control][ei]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xFB}) == 4);

    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("DI (0xF3)", "[control][di]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0xF3}) == 4);

    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("HLT (0x76)", "[control][hlt]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);

    CHECK(m.run(start, {0x76}) == 7);

    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("A halted CPU runs no more instructions", "[control][hlt]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    m.start(start, {0x76, 0x3C, 0x3C});   // HLT; INR A; INR A

    m.step();                             // HLT
    m.step();
    m.step();

    CpuState expected = start;
    expected.pc += 1;                     // A not incremented
    CHECK(m.state() == expected);
}


// ---------------------------------------------------------------------------
// Hardware interrupts
//
// The interrupting device supplies an RST n instruction. If interrupts are
// enabled, the CPU pushes PC, jumps to n * 8 and disables interrupts. If they
// are disabled, the request is ignored. Interrupts start disabled; EI enables
// them after the instruction that follows EI.
// ---------------------------------------------------------------------------

TEST_CASE("Interrupts are disabled at power-on", "[control][interrupt]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    m.start(start, {0x00});               // NOP

    m.step();
    m.interrupt(1);

    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);         // not taken
    CHECK(m.read(start.sp - 1) == 0x00);
    CHECK(m.read(start.sp - 2) == 0x00);
}

TEST_CASE("Interrupt after EI and one more instruction", "[control][interrupt][ei]") {
    TestMachine m;
    CpuState start = CpuState::initial(ALL);
    m.start(start, {0xFB, 0x00});         // EI; NOP

    m.step();                             // EI
    m.step();                             // NOP

    CHECK(m.interrupt(1) == 11);          // same as RST 1

    CpuState expected = start;
    expected.pc = 0x0008;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
    uint16_t ret = start.pc + 2;                    // return address: after EI; NOP
    CHECK(m.read(start.sp - 1) == (ret >> 8));      // high byte at SP-1
    CHECK(m.read(start.sp - 2) == (ret & 0xFF));    // low byte at SP-2
}

TEST_CASE("Interrupt jumps to n * 8 for each RST n", "[control][interrupt]") {
    for (uint8_t n = 0; n < 8; n++) {
        TestMachine m;
        CpuState start = CpuState::initial(0);
        m.start(start, {0xFB, 0x00});     // EI; NOP
        m.step();
        m.step();

        m.interrupt(n);

        CAPTURE(+n);
        CHECK(m.state().pc == n * 8);
    }
}

TEST_CASE("Interrupt right after EI is not taken", "[control][interrupt][ei]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    m.start(start, {0xFB});               // EI

    m.step();
    m.interrupt(1);

    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}

TEST_CASE("DI blocks interrupts", "[control][interrupt][di]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    m.start(start, {0xFB, 0x00, 0xF3});   // EI; NOP; DI

    m.step();
    m.step();
    m.step();
    m.interrupt(1);

    CpuState expected = start;
    expected.pc += 3;
    CHECK(m.state() == expected);
}

TEST_CASE("Taking an interrupt disables further interrupts", "[control][interrupt]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    m.start(start, {0xFB, 0x00});         // EI; NOP
    m.step();
    m.step();

    m.interrupt(1);                       // taken
    m.interrupt(2);                       // ignored

    CpuState expected = start;
    expected.pc = 0x0008;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
}

TEST_CASE("An interrupt wakes a halted CPU", "[control][interrupt][hlt]") {
    TestMachine m;
    m.write(0x0038, 0x47);                // MOV B,A at the RST 7 handler
    CpuState start = CpuState::initial(0);
    m.start(start, {0xFB, 0x76});         // EI; HLT

    m.step();                             // EI
    m.step();                             // HLT
    m.step();                             // still halted

    CHECK(m.interrupt(7) == 11);

    CpuState expected = start;
    expected.pc = 0x0038;
    expected.sp = start.sp - 2;
    CHECK(m.state() == expected);
    uint16_t ret = start.pc + 2;                    // return address: after EI; HLT
    CHECK(m.read(start.sp - 1) == (ret >> 8));      // high byte at SP-1
    CHECK(m.read(start.sp - 2) == (ret & 0xFF));    // low byte at SP-2

    m.step();                             // MOV B,A runs
    expected.pc = 0x0039;
    expected.b = start.a;
    CHECK(m.state() == expected);
}

TEST_CASE("HLT with interrupts disabled stays halted", "[control][interrupt][hlt]") {
    TestMachine m;
    CpuState start = CpuState::initial(0);
    m.start(start, {0x76, 0x3C});         // HLT; INR A

    m.step();
    m.interrupt(7);                       // ignored
    m.step();

    CpuState expected = start;
    expected.pc += 1;
    CHECK(m.state() == expected);
}
