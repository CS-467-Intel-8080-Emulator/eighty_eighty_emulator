// Short 8080 programs. Each one uses several instructions together (loops,
// subroutines, memory, stack, interrupts) and checks the end result, as a
// check that instructions combine correctly, not only one at a time.
//
// Programs load at 0100 with SP=2400. Most end with HLT; the interrupt
// program instead loops forever and is stepped a fixed number of times, and
// the decimal examples are stepped one instruction at a time to check each
// step.
// Addresses in the listings are hex; operands are written low byte first.

#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "test_machine.hpp"

using namespace spec;
using namespace spec::flag;

namespace {

CpuState program_start() {
    CpuState s = CpuState::initial();    // flags clear
    s.pc = 0x0100;                       // the listings use absolute addresses
    s.sp = 0x2400;
    return s;
}

// Steps until PC == end_pc (the address just past the final HLT).
// Returns the total cycles. Fails if it takes more than max_steps.
uint32_t run_to(TestMachine& m, uint16_t end_pc, int max_steps = 1000) {
    uint32_t cycles = 0;
    for (int i = 0; i < max_steps; i++) {
        cycles += m.step();
        if (m.state().pc == end_pc)
            return cycles;
    }
    FAIL("program did not reach PC " << end_pc << " within " << max_steps << " steps");
    return cycles;
}

// Runs one instruction, then checks A and the five flags
void step_and_check(TestMachine& m, const char* what, uint8_t a, uint8_t flags) {
    INFO(what);
    m.step();
    CpuState s = m.state();
    CHECK(s.a == a);
    CHECK((s.flags & ALL) == flags);
}

// The ALP's multibyte addition routine MADD (p. 55), with alu_op in the loop:
// ADC M (8E) adds, SBB M (9E) subtracts. FIRST is at 0200 and SECND at 0210,
// each stored low byte first; the result replaces FIRST.
//
// The manual's listing keeps the byte count in C, but C is also the low byte
// of the FIRST pointer (LXI B,FIRST), so the printed loop can't count both.
// Here the count is in E instead; nothing else changes.
//
// 0100  01 00 02   LXI B,0200    BC = FIRST
// 0103  21 10 02   LXI H,0210    HL = SECND
// 0106  1E nn      MVI E,count
// 0108  AF         XRA A         clear carry
// 0109  0A         loop: LDAX B
// 010A  8E / 9E    ADC M / SBB M
// 010B  02         STAX B
// 010C  1D         DCR E
// 010D  CA 15 01   JZ done
// 0110  03         INX B
// 0111  23         INX H
// 0112  C3 09 01   JMP loop
// 0115  76         done: HLT
void load_multibyte(TestMachine& m, uint8_t alu_op, uint8_t count) {
    m.start(program_start(),
            {0x01, 0x00, 0x02, 0x21, 0x10, 0x02, 0x1E, count, 0xAF,
             0x0A, alu_op, 0x02, 0x1D, 0xCA, 0x15, 0x01, 0x03, 0x23,
             0xC3, 0x09, 0x01, 0x76});
}

}  // namespace


TEST_CASE("Count-down loop: DCR and JNZ", "[program]") {
    // 0100  06 05      MVI B,05
    // 0102  05         loop: DCR B
    // 0103  C2 02 01   JNZ loop
    // 0106  76         HLT
    TestMachine m;
    m.start(program_start(), {0x06, 0x05, 0x05, 0xC2, 0x02, 0x01, 0x76});

    // MVI 7 + 5 * (DCR 5 + JNZ 10) + HLT 7
    CHECK(run_to(m, 0x0107) == 89);

    CpuState s = m.state();
    CHECK(s.b == 0x00);
    CHECK((s.flags & Z) != 0);
}

TEST_CASE("Sum 1 to 10: ADD, DCR, JNZ", "[program]") {
    // 0100  3E 00      MVI A,00
    // 0102  06 0A      MVI B,0A
    // 0104  80         loop: ADD B
    // 0105  05         DCR B
    // 0106  C2 04 01   JNZ loop
    // 0109  76         HLT
    TestMachine m;
    m.start(program_start(),
            {0x3E, 0x00, 0x06, 0x0A, 0x80, 0x05, 0xC2, 0x04, 0x01, 0x76});

    // 2 * MVI 7 + 10 * (ADD 4 + DCR 5 + JNZ 10) + HLT 7
    CHECK(run_to(m, 0x010A) == 211);

    CpuState s = m.state();
    CHECK(s.a == 55);                     // 0x37
    CHECK(s.b == 0);
}

TEST_CASE("16-bit add with carry: ADI then ACI", "[program]") {
    // 12FF + 0101 = 1400, low bytes first, carry into the high bytes
    // 0100  3E FF      MVI A,FF
    // 0102  C6 01      ADI 01      A=00, CY=1
    // 0104  6F         MOV L,A
    // 0105  3E 12      MVI A,12    (MVI keeps CY)
    // 0107  CE 01      ACI 01      A=12+01+1=14
    // 0109  67         MOV H,A
    // 010A  76         HLT
    TestMachine m;
    m.start(program_start(),
            {0x3E, 0xFF, 0xC6, 0x01, 0x6F, 0x3E, 0x12, 0xCE, 0x01, 0x67, 0x76});

    run_to(m, 0x010B);

    CpuState s = m.state();
    CHECK(s.hl() == 0x1400);
    CHECK((s.flags & CY) == 0);           // no carry out of the high byte
}

TEST_CASE("Block copy: LDAX, MOV M,A, INX, DCR, JNZ", "[program]") {
    // Copies 4 bytes from 2000 to 2100
    // 0100  11 00 20   LXI D,2000
    // 0103  21 00 21   LXI H,2100
    // 0106  06 04      MVI B,04
    // 0108  1A         loop: LDAX D
    // 0109  77         MOV M,A
    // 010A  13         INX D
    // 010B  23         INX H
    // 010C  05         DCR B
    // 010D  C2 08 01   JNZ loop
    // 0110  76         HLT
    TestMachine m;
    m.write(0x2000, 0xDE);
    m.write(0x2001, 0xAD);
    m.write(0x2002, 0xBE);
    m.write(0x2003, 0xEF);
    m.start(program_start(),
            {0x11, 0x00, 0x20, 0x21, 0x00, 0x21, 0x06, 0x04,
             0x1A, 0x77, 0x13, 0x23, 0x05, 0xC2, 0x08, 0x01, 0x76});

    run_to(m, 0x0111);

    CHECK(m.read(0x2100) == 0xDE);
    CHECK(m.read(0x2101) == 0xAD);
    CHECK(m.read(0x2102) == 0xBE);
    CHECK(m.read(0x2103) == 0xEF);
    CHECK(m.read(0x2104) == 0x00);        // nothing past the end
    CpuState s = m.state();
    CHECK(s.de() == 0x2004);
    CHECK(s.hl() == 0x2104);
    CHECK(s.b == 0);
}

TEST_CASE("Subroutine called twice: CALL and RET", "[program]") {
    // 0100  3E 03      MVI A,03
    // 0102  CD 10 01   CALL double
    // 0105  CD 10 01   CALL double
    // 0108  76         HLT
    // 0110  87         double: ADD A
    // 0111  C9         RET
    TestMachine m;
    m.write(0x0110, 0x87);
    m.write(0x0111, 0xC9);
    m.start(program_start(),
            {0x3E, 0x03, 0xCD, 0x10, 0x01, 0xCD, 0x10, 0x01, 0x76});

    // MVI 7 + 2 * (CALL 17 + ADD 4 + RET 10) + HLT 7
    CHECK(run_to(m, 0x0109) == 76);

    CpuState s = m.state();
    CHECK(s.a == 0x0C);
    CHECK(s.sp == 0x2400);                // stack balanced
}

TEST_CASE("Swap register pairs through the stack: PUSH and POP", "[program]") {
    // 0100  01 11 11   LXI B,1111
    // 0103  11 22 22   LXI D,2222
    // 0106  C5         PUSH B
    // 0107  D5         PUSH D
    // 0108  C1         POP B
    // 0109  D1         POP D
    // 010A  76         HLT
    TestMachine m;
    m.start(program_start(),
            {0x01, 0x11, 0x11, 0x11, 0x22, 0x22, 0xC5, 0xD5, 0xC1, 0xD1, 0x76});

    // 2 * LXI 10 + 2 * PUSH 11 + 2 * POP 10 + HLT 7
    CHECK(run_to(m, 0x010B, 20) == 69);

    CpuState s = m.state();
    CHECK(s.bc() == 0x2222);
    CHECK(s.de() == 0x1111);
    CHECK(s.sp == 0x2400);
}

TEST_CASE("Interrupt handler runs and returns to the main loop", "[program][interrupt]") {
    // 0100  FB         EI
    // 0101  C3 01 01   loop: JMP loop
    // 0008  04         RST 1 handler: INR B
    // 0009  FB         EI
    // 000A  C9         RET
    TestMachine m;
    m.write(0x0008, 0x04);
    m.write(0x0009, 0xFB);
    m.write(0x000A, 0xC9);
    CpuState start = program_start();
    start.b = 0x00;                       // interrupt counter
    m.start(start, {0xFB, 0xC3, 0x01, 0x01});

    m.step();                             // EI
    m.step();                             // JMP (interrupts now on)

    m.interrupt(1);                       // pushes 0101, jumps to 0008
    m.step();                             // INR B
    m.step();                             // EI
    m.step();                             // RET
    CHECK(m.state().pc == 0x0101);
    CHECK(m.state().b == 0x01);
    CHECK(m.state().sp == 0x2400);

    m.step();                             // JMP (interrupts back on)
    m.interrupt(1);                       // second interrupt is taken too
    m.step();
    m.step();
    m.step();
    CHECK(m.state().pc == 0x0101);
    CHECK(m.state().b == 0x02);
    CHECK(m.state().sp == 0x2400);
}


// ---------------------------------------------------------------------------
// Worked examples from the ALP's programming chapter (pp. 55-57)
// ---------------------------------------------------------------------------

TEST_CASE("Multibyte addition example: 32AF8A + 84BA90 = B76A1A (ALP p. 55)", "[program]") {
    // The manual's DB lines and its memory table swap FIRST and SECND; the sum
    // is the same either way. This follows the table.
    TestMachine m;
    m.write(0x0200, 0x8A); m.write(0x0201, 0xAF); m.write(0x0202, 0x32);   // FIRST
    m.write(0x0210, 0x90); m.write(0x0211, 0xBA); m.write(0x0212, 0x84);   // SECND
    load_multibyte(m, 0x8E, 3);           // ADC M

    run_to(m, 0x0116);

    CHECK(m.read(0x0200) == 0x1A);        // 8A + 90 = 11A, carry out
    CHECK(m.read(0x0201) == 0x6A);        // AF + BA + 1 = 16A, carry out
    CHECK(m.read(0x0202) == 0xB7);        // 32 + 84 + 1 = B7
    CHECK(m.read(0x0210) == 0x90);        // SECND unchanged
    CHECK(m.read(0x0211) == 0xBA);
    CHECK(m.read(0x0212) == 0x84);
    CHECK((m.state().flags & CY) == 0);   // no carry out of the top byte
}

TEST_CASE("Multibyte subtraction example: 1301 - 0503 = 0DFE (ALP p. 56)", "[program]") {
    // The manual: "duplicate the multibyte addition routine ... changing the
    // ADC instruction to an SBB instruction."
    TestMachine m;
    m.write(0x0200, 0x01); m.write(0x0201, 0x13);                           // FIRST
    m.write(0x0210, 0x03); m.write(0x0211, 0x05);                           // SECND
    load_multibyte(m, 0x9E, 2);           // SBB M

    run_to(m, 0x0116);

    CHECK(m.read(0x0200) == 0xFE);        // 01 - 03 = FE, borrow (CY set)
    CHECK(m.read(0x0201) == 0x0D);        // 13 - 05 - 1 = 0D: the borrow was used
    CHECK((m.state().flags & CY) == 0);   // no borrow out of the top byte
}

TEST_CASE("Decimal addition example: 2985 + 4936 = 7921 (ALP p. 56)", "[program][daa]") {
    // The manual gives A, CY and AC after each step; S, Z and P follow the
    // general rules.
    // 0100  3E 85   MVI A,85
    // 0102  C6 36   ADI 36      low digits; carry starts clear
    // 0104  27      DAA
    // 0105  6F      MOV L,A     store 21
    // 0106  3E 29   MVI A,29
    // 0108  CE 49   ACI 49      high digits plus the carry from the low ones
    // 010A  27      DAA
    // 010B  67      MOV H,A     store 79
    TestMachine m;
    m.start(program_start(),
            {0x3E, 0x85, 0xC6, 0x36, 0x27, 0x6F, 0x3E, 0x29, 0xCE, 0x49, 0x27, 0x67});

    m.step();                                                         // MVI A,85
    step_and_check(m, "ADI 36: 85 + 36 = BB, CY and AC clear", 0xBB, S | P);
    step_and_check(m, "DAA: BB -> 21, CY set", 0x21, AC | P | CY);
    m.step();                                                         // MOV L,A
    m.step();                                                         // MVI A,29
    step_and_check(m, "ACI 49: 29 + 49 + 1 = 73, CY clear, AC set", 0x73, AC);
    step_and_check(m, "DAA: 73 -> 79, CY clear", 0x79, 0);
    m.step();                                                         // MOV H,A

    CHECK(m.state().hl() == 0x7921);
}

TEST_CASE("Decimal subtraction example: 4358 - 1362 = 2996 (ALP p. 57)", "[program][daa]") {
    // The manual's steps: CY = 1 (no borrow yet); for each pair of digits,
    // A = 99 + CY (the 100's or 99's complement base), subtract the
    // subtrahend digits, add the minuend digits, then DAA. After DAA, CY = 0
    // means a borrow occurred. The manual describes the steps without a
    // listing; this runs them with immediates. The manual gives A, CY and AC
    // where they matter; the other flags follow the general rules.
    // 0100  37      STC
    // 0101  3E 99   MVI A,99
    // 0103  CE 00   ACI 00
    // 0105  D6 62   SUI 62
    // 0107  C6 58   ADI 58
    // 0109  27      DAA
    // 010A  6F      MOV L,A     store 96
    // 010B  3E 99   MVI A,99
    // 010D  CE 00   ACI 00
    // 010F  D6 13   SUI 13
    // 0111  C6 43   ADI 43
    // 0113  27      DAA
    // 0114  67      MOV H,A     store 29
    TestMachine m;
    m.start(program_start(),
            {0x37, 0x3E, 0x99, 0xCE, 0x00, 0xD6, 0x62, 0xC6, 0x58, 0x27, 0x6F,
             0x3E, 0x99, 0xCE, 0x00, 0xD6, 0x13, 0xC6, 0x43, 0x27, 0x67});

    m.step();                                                         // STC
    m.step();                                                         // MVI A,99
    step_and_check(m, "ACI 00: 99 + 0 + 1 = 9A, CY cleared", 0x9A, S | P);
    step_and_check(m, "SUI 62: 9A - 62 = 38", 0x38, AC);
    step_and_check(m, "ADI 58: 38 + 58 = 90, CY clear, AC set", 0x90, S | AC | P);
    step_and_check(m, "DAA: 90 -> 96, CY clear (borrow)", 0x96, S | P);
    m.step();                                                         // MOV L,A
    m.step();                                                         // MVI A,99
    step_and_check(m, "ACI 00: 99 + 0 + 0 = 99", 0x99, S | P);
    step_and_check(m, "SUI 13: 99 - 13 = 86", 0x86, S | AC);
    step_and_check(m, "ADI 43: 86 + 43 = C9, CY and AC clear", 0xC9, S | P);
    step_and_check(m, "DAA: C9 -> 29, CY set (no borrow)", 0x29, CY);
    m.step();                                                         // MOV H,A

    CHECK(m.state().hl() == 0x2996);
}
