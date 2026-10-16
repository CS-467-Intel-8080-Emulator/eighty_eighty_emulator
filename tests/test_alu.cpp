// Unit tests for flags and arithmetic/logic instructions.
// "Manual example" values are from the Intel 8080 Assembly Language Programming Manual.

#include <cstdint>
#include <cstdio>
#include <initializer_list>

#include "../src/core/cpu.hpp"
#include "test_memory.hpp"

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
};
using T = CPUTestAccess;

// Flag masks
constexpr uint8_t CY = 0x01;
constexpr uint8_t P  = 0x04;
constexpr uint8_t AC = 0x10;
constexpr uint8_t Z  = 0x40;
constexpr uint8_t S  = 0x80;
constexpr uint8_t ALL_FLAGS = CY | P | AC | Z | S;

static int failures = 0;
static int checks = 0;

#define CHECK_EQ(actual, expected)                                              \
    do {                                                                        \
        checks++;                                                               \
        unsigned long long _a = (unsigned long long)(actual);                   \
        unsigned long long _e = (unsigned long long)(expected);                 \
        if (_a != _e) {                                                         \
            failures++;                                                         \
            std::printf("FAIL %s:%d  %s == 0x%llX, expected 0x%llX\n",          \
                        __FILE__, __LINE__, #actual, _a, _e);                   \
        }                                                                       \
    } while (0)

// Compares S, Z, AC, P, CY only
#define CHECK_FLAGS(cpu, expected) CHECK_EQ(T::flags(cpu) & ALL_FLAGS, (expected))

// Clears memory and loads program at 0x0000
static CPU make_cpu(std::initializer_list<uint8_t> program) {
    test_ram.fill(0);
    uint16_t addr = 0;
    for (uint8_t byte : program)
        test_ram[addr++] = byte;

    CPU cpu;
    T::pc(cpu) = 0;
    T::flags(cpu) = 0;
    return cpu;
}

static Memory mem;


// --- Addition ---

static void test_add() {
    // Manual example: 0x6C + 0x2E = 0x9A
    CPU cpu = make_cpu({0x80});            // ADD B
    T::a(cpu) = 0x6C;
    T::b(cpu) = 0x2E;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x9A);
    CHECK_EQ(T::b(cpu), 0x2E);
    CHECK_FLAGS(cpu, S | AC | P);

    // Overflow wraps to 0x00, sets CY
    cpu = make_cpu({0x80});                // ADD B
    T::a(cpu) = 0xFF;
    T::b(cpu) = 0x01;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P | CY);

    cpu = make_cpu({0x86});                // ADD M
    T::a(cpu) = 0x05;
    T::h(cpu) = 0x20;
    T::l(cpu) = 0x00;
    test_ram[0x2000] = 0x10;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0x15);
    CHECK_FLAGS(cpu, 0);

    cpu = make_cpu({0x84});                // ADD H
    T::a(cpu) = 0x01;
    T::h(cpu) = 0x02;
    T::l(cpu) = 0x10;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x03);
    CHECK_FLAGS(cpu, P);
}

static void test_adc() {
    // Manual example: 0x42 + 0x3D + carry = 0x80
    CPU cpu = make_cpu({0x89});            // ADC C
    T::a(cpu) = 0x42;
    T::c(cpu) = 0x3D;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x80);
    CHECK_FLAGS(cpu, S | AC);

    // Same operands without carry = 0x7F
    cpu = make_cpu({0x89});                // ADC C
    T::a(cpu) = 0x42;
    T::c(cpu) = 0x3D;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x7F);
    CHECK_FLAGS(cpu, 0);

    // 0x00 + 0xFF + carry = 0x00
    cpu = make_cpu({0x88});                // ADC B
    T::a(cpu) = 0x00;
    T::b(cpu) = 0xFF;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P | CY);
}


// --- Subtraction and compare ---

static void test_sub() {
    // Manual example: SUB A clears A and sets AC
    CPU cpu = make_cpu({0x97});            // SUB A
    T::a(cpu) = 0x3E;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P);

    // Borrow: 0x01 - 0x02 = 0xFF
    cpu = make_cpu({0x90});                // SUB B
    T::a(cpu) = 0x01;
    T::b(cpu) = 0x02;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0xFF);
    CHECK_FLAGS(cpu, S | P | CY);

    // Half-borrow clears AC: 0x10 - 0x01 = 0x0F
    cpu = make_cpu({0x90});                // SUB B
    T::a(cpu) = 0x10;
    T::b(cpu) = 0x01;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x0F);
    CHECK_FLAGS(cpu, P);
}

static void test_sbb() {
    // Manual example: 0x04 - 0x02 - borrow = 0x01
    CPU cpu = make_cpu({0x9D});            // SBB L
    T::a(cpu) = 0x04;
    T::l(cpu) = 0x02;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x01);
    CHECK_FLAGS(cpu, AC);

    // 0x00 - 0x00 - borrow = 0xFF
    cpu = make_cpu({0x98});                // SBB B
    T::a(cpu) = 0x00;
    T::b(cpu) = 0x00;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0xFF);
    CHECK_FLAGS(cpu, S | P | CY);
}

static void test_cmp() {
    // A > operand: A and E unchanged
    CPU cpu = make_cpu({0xBB});            // CMP E
    T::a(cpu) = 0x0A;
    T::e(cpu) = 0x05;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x0A);
    CHECK_EQ(T::e(cpu), 0x05);
    CHECK_FLAGS(cpu, AC | P);

    // A < operand: CY set
    cpu = make_cpu({0xBB});                // CMP E
    T::a(cpu) = 0x02;
    T::e(cpu) = 0x05;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x02);
    CHECK_FLAGS(cpu, S | CY);

    // A == operand: Z set
    cpu = make_cpu({0xBB});                // CMP E
    T::a(cpu) = 0x05;
    T::e(cpu) = 0x05;
    cpu.step(mem);
    CHECK_FLAGS(cpu, Z | AC | P);
}


// --- Logic ---

static void test_ana() {
    // AC is bit 3 of (A | operand)
    CPU cpu = make_cpu({0xA1});            // ANA C
    T::a(cpu) = 0xFC;
    T::c(cpu) = 0x0F;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x0C);
    CHECK_FLAGS(cpu, AC | P);

    // Bit 3 clear in both: AC clear
    cpu = make_cpu({0xA0});                // ANA B
    T::a(cpu) = 0xF0;
    T::b(cpu) = 0x30;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x30);
    CHECK_FLAGS(cpu, P);

    // AC from the operands, not the result
    cpu = make_cpu({0xA0});                // ANA B
    T::a(cpu) = 0x08;
    T::b(cpu) = 0x00;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P);
}

static void test_xra_ora() {
    // XRA A clears A, CY, AC
    CPU cpu = make_cpu({0xAF});            // XRA A
    T::a(cpu) = 0x5A;
    T::flags(cpu) = CY | AC;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | P);

    // ORA: 0x33 | 0x0F = 0x3F
    cpu = make_cpu({0xB1});                // ORA C
    T::a(cpu) = 0x33;
    T::c(cpu) = 0x0F;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x3F);
    CHECK_FLAGS(cpu, P);

    // Even parity
    cpu = make_cpu({0xB0});                // ORA B
    T::a(cpu) = 0x01;
    T::b(cpu) = 0x02;
    T::flags(cpu) = P;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x03);
    CHECK_FLAGS(cpu, P);

    // Odd parity
    cpu = make_cpu({0xB0});                // ORA B
    T::a(cpu) = 0x01;
    T::b(cpu) = 0x00;
    T::flags(cpu) = P;
    cpu.step(mem);
    CHECK_FLAGS(cpu, 0);
}


// --- Increment / decrement ---

static void test_inr_dcr() {
    // INR wraps to 0x00, CY unchanged
    CPU cpu = make_cpu({0x04});            // INR B
    T::b(cpu) = 0xFF;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 5);
    CHECK_EQ(T::b(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P | CY);

    // INR wrap leaves CY clear
    cpu = make_cpu({0x04});                // INR B
    T::b(cpu) = 0xFF;
    cpu.step(mem);
    CHECK_FLAGS(cpu, Z | AC | P);

    // INR E leaves D unchanged
    cpu = make_cpu({0x1C});                // INR E
    T::d(cpu) = 0x00;
    T::e(cpu) = 0x0F;
    cpu.step(mem);
    CHECK_EQ(T::e(cpu), 0x10);
    CHECK_EQ(T::d(cpu), 0x00);
    CHECK_FLAGS(cpu, AC);

    cpu = make_cpu({0x34});                // INR M
    T::h(cpu) = 0x20;
    T::l(cpu) = 0x10;
    test_ram[0x2010] = 0x41;
    CHECK_EQ(cpu.step(mem), 10);
    CHECK_EQ(test_ram[0x2010], 0x42);
    CHECK_FLAGS(cpu, P);

    // DCR wraps to 0xFF, CY unchanged
    cpu = make_cpu({0x0D});                // DCR C
    T::c(cpu) = 0x00;
    CHECK_EQ(cpu.step(mem), 5);
    CHECK_EQ(T::c(cpu), 0xFF);
    CHECK_FLAGS(cpu, S | P);

    // Half-borrow clears AC: 0x10 -> 0x0F
    cpu = make_cpu({0x3D});                // DCR A
    T::a(cpu) = 0x10;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x0F);
    CHECK_FLAGS(cpu, P | CY);

    // No half-borrow sets AC: 0x05 -> 0x04
    cpu = make_cpu({0x3D});                // DCR A
    T::a(cpu) = 0x05;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x04);
    CHECK_FLAGS(cpu, AC);

    cpu = make_cpu({0x35});                // DCR M
    T::h(cpu) = 0x20;
    T::l(cpu) = 0x10;
    test_ram[0x2010] = 0x01;
    CHECK_EQ(cpu.step(mem), 10);
    CHECK_EQ(test_ram[0x2010], 0x00);
    CHECK_FLAGS(cpu, Z | AC | P);
}


// --- 16-bit add ---

static void test_dad() {
    // Manual example: 0xA17B + 0x339F = 0xD51A
    CPU cpu = make_cpu({0x09});            // DAD B
    T::h(cpu) = 0xA1;
    T::l(cpu) = 0x7B;
    T::b(cpu) = 0x33;
    T::c(cpu) = 0x9F;
    T::flags(cpu) = Z | S;
    CHECK_EQ(cpu.step(mem), 10);
    CHECK_EQ(T::h(cpu), 0xD5);
    CHECK_EQ(T::l(cpu), 0x1A);
    CHECK_FLAGS(cpu, Z | S);

    // Overflow sets CY
    cpu = make_cpu({0x19});                // DAD D
    T::h(cpu) = 0xFF;
    T::l(cpu) = 0xFF;
    T::d(cpu) = 0x00;
    T::e(cpu) = 0x01;
    cpu.step(mem);
    CHECK_EQ(T::h(cpu), 0x00);
    CHECK_EQ(T::l(cpu), 0x00);
    CHECK_FLAGS(cpu, CY);

    // No overflow clears CY
    cpu = make_cpu({0x09});                // DAD B
    T::h(cpu) = 0x10;
    T::l(cpu) = 0x00;
    T::b(cpu) = 0x00;
    T::c(cpu) = 0x01;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_EQ(T::h(cpu), 0x10);
    CHECK_EQ(T::l(cpu), 0x01);
    CHECK_FLAGS(cpu, 0);

    // DAD H doubles HL
    cpu = make_cpu({0x29});                // DAD H
    T::h(cpu) = 0x12;
    T::l(cpu) = 0x34;
    cpu.step(mem);
    CHECK_EQ(T::h(cpu), 0x24);
    CHECK_EQ(T::l(cpu), 0x68);

    cpu = make_cpu({0x39});                // DAD SP
    T::h(cpu) = 0x00;
    T::l(cpu) = 0x01;
    T::sp(cpu) = 0x2400;
    cpu.step(mem);
    CHECK_EQ(T::h(cpu), 0x24);
    CHECK_EQ(T::l(cpu), 0x01);
}


// --- Decimal adjust ---

static void test_daa() {
    // Manual example: 0x9B -> 0x01, AC and CY set
    CPU cpu = make_cpu({0x27});            // DAA
    T::a(cpu) = 0x9B;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0x01);
    CHECK_FLAGS(cpu, AC | CY);

    // BCD 38 + 45 = 83
    cpu = make_cpu({0x80, 0x27});          // ADD B; DAA
    T::a(cpu) = 0x38;
    T::b(cpu) = 0x45;
    cpu.step(mem);
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x83);
    CHECK_FLAGS(cpu, S | AC);

    // BCD 99 + 01 = 00, CY set
    cpu = make_cpu({0xC6, 0x01, 0x27});    // ADI 01; DAA
    T::a(cpu) = 0x99;
    cpu.step(mem);
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P | CY);

    // BCD 99 + 99 = 98, CY set: DAA uses incoming AC and CY
    cpu = make_cpu({0x80, 0x27});          // ADD B; DAA
    T::a(cpu) = 0x99;
    T::b(cpu) = 0x99;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x32);
    CHECK_FLAGS(cpu, AC | CY);
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x98);
    CHECK_FLAGS(cpu, S | CY);

    // High digit > 9
    cpu = make_cpu({0x27});                // DAA
    T::a(cpu) = 0xA0;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | P | CY);
}


// --- Immediates ---

static void test_immediates() {
    // ADI: manual example 0x14 + 0x42 = 0x56
    CPU cpu = make_cpu({0xC6, 0x42});      // ADI 42
    T::a(cpu) = 0x14;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0x56);
    CHECK_EQ(T::pc(cpu), 2);
    CHECK_FLAGS(cpu, P);

    cpu = make_cpu({0xCE, 0x01});          // ACI 01
    T::a(cpu) = 0x01;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0x03);
    CHECK_FLAGS(cpu, P);

    cpu = make_cpu({0xD6, 0x01});          // SUI 01
    T::a(cpu) = 0x00;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0xFF);
    CHECK_FLAGS(cpu, S | P | CY);

    cpu = make_cpu({0xDE, 0x01});          // SBI 01
    T::a(cpu) = 0x05;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0x03);
    CHECK_FLAGS(cpu, AC | P);

    cpu = make_cpu({0xE6, 0x0F});          // ANI 0F
    T::a(cpu) = 0x3A;
    T::flags(cpu) = CY;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0x0A);
    CHECK_FLAGS(cpu, AC | P);

    cpu = make_cpu({0xEE, 0xFF});          // XRI FF
    T::a(cpu) = 0x0F;
    T::flags(cpu) = CY | AC;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0xF0);
    CHECK_FLAGS(cpu, S | P);

    cpu = make_cpu({0xF6, 0x80});          // ORI 80
    T::a(cpu) = 0x00;
    T::flags(cpu) = CY | AC;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0x80);
    CHECK_FLAGS(cpu, S);

    // CPI: manual example 0x4A vs 0x40
    cpu = make_cpu({0xFE, 0x40});          // CPI 40
    T::a(cpu) = 0x4A;
    CHECK_EQ(cpu.step(mem), 7);
    CHECK_EQ(T::a(cpu), 0x4A);
    CHECK_FLAGS(cpu, AC | P);
}


// --- Rotates and carry operations ---

static void test_rotates() {
    // Manual examples
    CPU cpu = make_cpu({0x07});            // RLC
    T::a(cpu) = 0xF2;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0xE5);
    CHECK_FLAGS(cpu, CY);

    cpu = make_cpu({0x0F});                // RRC
    T::a(cpu) = 0xF2;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x79);
    CHECK_FLAGS(cpu, 0);

    cpu = make_cpu({0x17});                // RAL
    T::a(cpu) = 0xB5;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0x6A);
    CHECK_FLAGS(cpu, CY);

    cpu = make_cpu({0x1F});                // RAR
    T::a(cpu) = 0x6A;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_EQ(T::a(cpu), 0xB5);
    CHECK_FLAGS(cpu, 0);

    // RLC, RRC, RAL, RAR, STC, CMC leave S, Z, AC, P unchanged
    for (int opcode : {0x07, 0x0F, 0x17, 0x1F, 0x37, 0x3F}) {
        cpu = make_cpu({(uint8_t)opcode});
        T::a(cpu) = 0x00;
        T::flags(cpu) = S | Z | AC | P;
        cpu.step(mem);
        CHECK_EQ(T::flags(cpu) & (S | Z | AC | P), S | Z | AC | P);
    }
}

static void test_cma_stc_cmc() {
    CPU cpu = make_cpu({0x2F});            // CMA
    T::a(cpu) = 0x51;
    T::flags(cpu) = Z;
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_EQ(T::a(cpu), 0xAE);
    CHECK_FLAGS(cpu, Z);

    cpu = make_cpu({0x37, 0x3F, 0x3F});    // STC; CMC; CMC
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_FLAGS(cpu, CY);
    CHECK_EQ(cpu.step(mem), 4);
    CHECK_FLAGS(cpu, 0);
    cpu.step(mem);
    CHECK_FLAGS(cpu, CY);
}


// --- Cycle counts ---

static void test_alu_cycles() {
    // 0x80 - 0xBF: M form 7 cycles, register forms 4
    for (int opcode = 0x80; opcode <= 0xBF; opcode++) {
        CPU cpu = make_cpu({(uint8_t)opcode});
        T::h(cpu) = 0x20;
        T::l(cpu) = 0x00;
        CHECK_EQ(cpu.step(mem), (opcode & 0x07) == 6 ? 7 : 4);
    }
}


// --- Flag byte layout ---

static void test_unused_flag_bits_preserved() {
    // Bits 1, 3, 5 unchanged
    CPU cpu = make_cpu({0x80});            // ADD B
    T::flags(cpu) = 0x02;
    T::a(cpu) = 0x01;
    T::b(cpu) = 0x01;
    cpu.step(mem);
    CHECK_EQ(T::flags(cpu) & 0x2A, 0x02);
}


int main() {
    test_add();
    test_adc();
    test_sub();
    test_sbb();
    test_cmp();
    test_ana();
    test_xra_ora();
    test_inr_dcr();
    test_dad();
    test_daa();
    test_immediates();
    test_rotates();
    test_cma_stc_cmc();
    test_alu_cycles();
    test_unused_flag_bits_preserved();

    std::printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
