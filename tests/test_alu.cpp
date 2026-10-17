// Unit tests for flags and arithmetic/logic instructions.
// "Manual example" values are from the Intel 8080 Assembly Language Programming Manual.

#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>

#include <catch2/catch_test_macros.hpp>

#include "cpu_test_helpers.hpp"

// Value that prints as 0xNN in failure output
struct Hex {
    unsigned value;
    explicit Hex(unsigned v) : value(v) {}
    bool operator==(const Hex& other) const { return value == other.value; }
};

inline std::ostream& operator<<(std::ostream& os, const Hex& h) {
    std::ios_base::fmtflags saved = os.flags();
    os << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << h.value;
    os.flags(saved);
    return os;
}

#define CHECK_HEX(actual, expected) CHECK(Hex(actual) == Hex(expected))

// Compares S, Z, AC, P, CY only
#define CHECK_FLAGS(cpu, expected) CHECK_HEX(T::flags(cpu) & ALL_FLAGS, (expected))

static Memory mem;


// --- Addition ---

TEST_CASE("ADD", "[alu]") {
    // Manual example: 0x6C + 0x2E = 0x9A
    CPU cpu = make_cpu({0x80});            // ADD B
    T::a(cpu) = 0x6C;
    T::b(cpu) = 0x2E;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x9A);
    CHECK_HEX(T::b(cpu), 0x2E);
    CHECK_FLAGS(cpu, S | AC | P);

    // Overflow wraps to 0x00, sets CY
    cpu = make_cpu({0x80});                // ADD B
    T::a(cpu) = 0xFF;
    T::b(cpu) = 0x01;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P | CY);

    cpu = make_cpu({0x86});                // ADD M
    T::a(cpu) = 0x05;
    T::h(cpu) = 0x20;
    T::l(cpu) = 0x00;
    test_ram[0x2000] = 0x10;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0x15);
    CHECK_FLAGS(cpu, 0);

    cpu = make_cpu({0x84});                // ADD H
    T::a(cpu) = 0x01;
    T::h(cpu) = 0x02;
    T::l(cpu) = 0x10;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x03);
    CHECK_FLAGS(cpu, P);
}

TEST_CASE("ADC", "[alu]") {
    // Manual example: 0x42 + 0x3D + carry = 0x80
    CPU cpu = make_cpu({0x89});            // ADC C
    T::a(cpu) = 0x42;
    T::c(cpu) = 0x3D;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x80);
    CHECK_FLAGS(cpu, S | AC);

    // Same operands without carry = 0x7F
    cpu = make_cpu({0x89});                // ADC C
    T::a(cpu) = 0x42;
    T::c(cpu) = 0x3D;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x7F);
    CHECK_FLAGS(cpu, 0);

    // 0x00 + 0xFF + carry = 0x00
    cpu = make_cpu({0x88});                // ADC B
    T::a(cpu) = 0x00;
    T::b(cpu) = 0xFF;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P | CY);
}


// --- Subtraction and compare ---

TEST_CASE("SUB", "[alu]") {
    // Manual example: SUB A clears A and sets AC
    CPU cpu = make_cpu({0x97});            // SUB A
    T::a(cpu) = 0x3E;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P);

    // Borrow: 0x01 - 0x02 = 0xFF
    cpu = make_cpu({0x90});                // SUB B
    T::a(cpu) = 0x01;
    T::b(cpu) = 0x02;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0xFF);
    CHECK_FLAGS(cpu, S | P | CY);

    // Half-borrow clears AC: 0x10 - 0x01 = 0x0F
    cpu = make_cpu({0x90});                // SUB B
    T::a(cpu) = 0x10;
    T::b(cpu) = 0x01;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x0F);
    CHECK_FLAGS(cpu, P);
}

TEST_CASE("SBB", "[alu]") {
    // Manual example: 0x04 - 0x02 - borrow = 0x01
    CPU cpu = make_cpu({0x9D});            // SBB L
    T::a(cpu) = 0x04;
    T::l(cpu) = 0x02;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x01);
    CHECK_FLAGS(cpu, AC);

    // 0x00 - 0x00 - borrow = 0xFF
    cpu = make_cpu({0x98});                // SBB B
    T::a(cpu) = 0x00;
    T::b(cpu) = 0x00;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0xFF);
    CHECK_FLAGS(cpu, S | P | CY);
}

TEST_CASE("CMP", "[alu]") {
    // A > operand: A and E unchanged
    CPU cpu = make_cpu({0xBB});            // CMP E
    T::a(cpu) = 0x0A;
    T::e(cpu) = 0x05;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x0A);
    CHECK_HEX(T::e(cpu), 0x05);
    CHECK_FLAGS(cpu, AC | P);

    // A < operand: CY set
    cpu = make_cpu({0xBB});                // CMP E
    T::a(cpu) = 0x02;
    T::e(cpu) = 0x05;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x02);
    CHECK_FLAGS(cpu, S | CY);

    // A == operand: Z set
    cpu = make_cpu({0xBB});                // CMP E
    T::a(cpu) = 0x05;
    T::e(cpu) = 0x05;
    cpu.step(mem);
    CHECK_FLAGS(cpu, Z | AC | P);
}


// --- Logic ---

TEST_CASE("ANA", "[alu]") {
    // AC is bit 3 of (A | operand)
    CPU cpu = make_cpu({0xA1});            // ANA C
    T::a(cpu) = 0xFC;
    T::c(cpu) = 0x0F;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x0C);
    CHECK_FLAGS(cpu, AC | P);

    // Bit 3 clear in both: AC clear
    cpu = make_cpu({0xA0});                // ANA B
    T::a(cpu) = 0xF0;
    T::b(cpu) = 0x30;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x30);
    CHECK_FLAGS(cpu, P);

    // AC from the operands, not the result
    cpu = make_cpu({0xA0});                // ANA B
    T::a(cpu) = 0x08;
    T::b(cpu) = 0x00;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P);
}

TEST_CASE("XRA and ORA", "[alu]") {
    // XRA A clears A, CY, AC
    CPU cpu = make_cpu({0xAF});            // XRA A
    T::a(cpu) = 0x5A;
    T::flags(cpu) = CY | AC;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | P);

    // ORA: 0x33 | 0x0F = 0x3F
    cpu = make_cpu({0xB1});                // ORA C
    T::a(cpu) = 0x33;
    T::c(cpu) = 0x0F;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x3F);
    CHECK_FLAGS(cpu, P);

    // Even parity
    cpu = make_cpu({0xB0});                // ORA B
    T::a(cpu) = 0x01;
    T::b(cpu) = 0x02;
    T::flags(cpu) = P;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x03);
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

TEST_CASE("INR and DCR", "[alu]") {
    // INR wraps to 0x00, CY unchanged
    CPU cpu = make_cpu({0x04});            // INR B
    T::b(cpu) = 0xFF;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 5);
    CHECK_HEX(T::b(cpu), 0x00);
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
    CHECK_HEX(T::e(cpu), 0x10);
    CHECK_HEX(T::d(cpu), 0x00);
    CHECK_FLAGS(cpu, AC);

    cpu = make_cpu({0x34});                // INR M
    T::h(cpu) = 0x20;
    T::l(cpu) = 0x10;
    test_ram[0x2010] = 0x41;
    CHECK(cpu.step(mem) == 10);
    CHECK_HEX(test_ram[0x2010], 0x42);
    CHECK_FLAGS(cpu, P);

    // DCR wraps to 0xFF, CY unchanged
    cpu = make_cpu({0x0D});                // DCR C
    T::c(cpu) = 0x00;
    CHECK(cpu.step(mem) == 5);
    CHECK_HEX(T::c(cpu), 0xFF);
    CHECK_FLAGS(cpu, S | P);

    // Half-borrow clears AC: 0x10 -> 0x0F
    cpu = make_cpu({0x3D});                // DCR A
    T::a(cpu) = 0x10;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x0F);
    CHECK_FLAGS(cpu, P | CY);

    // No half-borrow sets AC: 0x05 -> 0x04
    cpu = make_cpu({0x3D});                // DCR A
    T::a(cpu) = 0x05;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x04);
    CHECK_FLAGS(cpu, AC);

    cpu = make_cpu({0x35});                // DCR M
    T::h(cpu) = 0x20;
    T::l(cpu) = 0x10;
    test_ram[0x2010] = 0x01;
    CHECK(cpu.step(mem) == 10);
    CHECK_HEX(test_ram[0x2010], 0x00);
    CHECK_FLAGS(cpu, Z | AC | P);
}


// --- 16-bit add ---

TEST_CASE("DAD", "[alu]") {
    // Manual example: 0xA17B + 0x339F = 0xD51A
    CPU cpu = make_cpu({0x09});            // DAD B
    T::h(cpu) = 0xA1;
    T::l(cpu) = 0x7B;
    T::b(cpu) = 0x33;
    T::c(cpu) = 0x9F;
    T::flags(cpu) = Z | S;
    CHECK(cpu.step(mem) == 10);
    CHECK_HEX(T::h(cpu), 0xD5);
    CHECK_HEX(T::l(cpu), 0x1A);
    CHECK_FLAGS(cpu, Z | S);

    // Overflow sets CY
    cpu = make_cpu({0x19});                // DAD D
    T::h(cpu) = 0xFF;
    T::l(cpu) = 0xFF;
    T::d(cpu) = 0x00;
    T::e(cpu) = 0x01;
    cpu.step(mem);
    CHECK_HEX(T::h(cpu), 0x00);
    CHECK_HEX(T::l(cpu), 0x00);
    CHECK_FLAGS(cpu, CY);

    // No overflow clears CY
    cpu = make_cpu({0x09});                // DAD B
    T::h(cpu) = 0x10;
    T::l(cpu) = 0x00;
    T::b(cpu) = 0x00;
    T::c(cpu) = 0x01;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_HEX(T::h(cpu), 0x10);
    CHECK_HEX(T::l(cpu), 0x01);
    CHECK_FLAGS(cpu, 0);

    // DAD H doubles HL
    cpu = make_cpu({0x29});                // DAD H
    T::h(cpu) = 0x12;
    T::l(cpu) = 0x34;
    cpu.step(mem);
    CHECK_HEX(T::h(cpu), 0x24);
    CHECK_HEX(T::l(cpu), 0x68);

    cpu = make_cpu({0x39});                // DAD SP
    T::h(cpu) = 0x00;
    T::l(cpu) = 0x01;
    T::sp(cpu) = 0x2400;
    cpu.step(mem);
    CHECK_HEX(T::h(cpu), 0x24);
    CHECK_HEX(T::l(cpu), 0x01);
}


// --- Decimal adjust ---

TEST_CASE("DAA", "[alu]") {
    // Manual example: 0x9B -> 0x01, AC and CY set
    CPU cpu = make_cpu({0x27});            // DAA
    T::a(cpu) = 0x9B;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0x01);
    CHECK_FLAGS(cpu, AC | CY);

    // BCD 38 + 45 = 83
    cpu = make_cpu({0x80, 0x27});          // ADD B; DAA
    T::a(cpu) = 0x38;
    T::b(cpu) = 0x45;
    cpu.step(mem);
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x83);
    CHECK_FLAGS(cpu, S | AC);

    // BCD 99 + 01 = 00, CY set
    cpu = make_cpu({0xC6, 0x01, 0x27});    // ADI 01; DAA
    T::a(cpu) = 0x99;
    cpu.step(mem);
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | AC | P | CY);

    // BCD 99 + 99 = 98, CY set: DAA uses incoming AC and CY
    cpu = make_cpu({0x80, 0x27});          // ADD B; DAA
    T::a(cpu) = 0x99;
    T::b(cpu) = 0x99;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x32);
    CHECK_FLAGS(cpu, AC | CY);
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x98);
    CHECK_FLAGS(cpu, S | CY);

    // High digit > 9
    cpu = make_cpu({0x27});                // DAA
    T::a(cpu) = 0xA0;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x00);
    CHECK_FLAGS(cpu, Z | P | CY);
}


// --- Immediates ---

TEST_CASE("Immediates", "[alu]") {
    // ADI: manual example 0x14 + 0x42 = 0x56
    CPU cpu = make_cpu({0xC6, 0x42});      // ADI 42
    T::a(cpu) = 0x14;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0x56);
    CHECK_HEX(T::pc(cpu), ORIGIN + 2);
    CHECK_FLAGS(cpu, P);

    cpu = make_cpu({0xCE, 0x01});          // ACI 01
    T::a(cpu) = 0x01;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0x03);
    CHECK_FLAGS(cpu, P);

    cpu = make_cpu({0xD6, 0x01});          // SUI 01
    T::a(cpu) = 0x00;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0xFF);
    CHECK_FLAGS(cpu, S | P | CY);

    cpu = make_cpu({0xDE, 0x01});          // SBI 01
    T::a(cpu) = 0x05;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0x03);
    CHECK_FLAGS(cpu, AC | P);

    cpu = make_cpu({0xE6, 0x0F});          // ANI 0F
    T::a(cpu) = 0x3A;
    T::flags(cpu) = CY;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0x0A);
    CHECK_FLAGS(cpu, AC | P);

    cpu = make_cpu({0xEE, 0xFF});          // XRI FF
    T::a(cpu) = 0x0F;
    T::flags(cpu) = CY | AC;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0xF0);
    CHECK_FLAGS(cpu, S | P);

    cpu = make_cpu({0xF6, 0x80});          // ORI 80
    T::a(cpu) = 0x00;
    T::flags(cpu) = CY | AC;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0x80);
    CHECK_FLAGS(cpu, S);

    // CPI: manual example 0x4A vs 0x40
    cpu = make_cpu({0xFE, 0x40});          // CPI 40
    T::a(cpu) = 0x4A;
    CHECK(cpu.step(mem) == 7);
    CHECK_HEX(T::a(cpu), 0x4A);
    CHECK_FLAGS(cpu, AC | P);
}


// --- Rotates and carry operations ---

TEST_CASE("Rotates", "[alu]") {
    // Manual examples
    CPU cpu = make_cpu({0x07});            // RLC
    T::a(cpu) = 0xF2;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0xE5);
    CHECK_FLAGS(cpu, CY);

    cpu = make_cpu({0x0F});                // RRC
    T::a(cpu) = 0xF2;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x79);
    CHECK_FLAGS(cpu, 0);

    cpu = make_cpu({0x17});                // RAL
    T::a(cpu) = 0xB5;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0x6A);
    CHECK_FLAGS(cpu, CY);

    cpu = make_cpu({0x1F});                // RAR
    T::a(cpu) = 0x6A;
    T::flags(cpu) = CY;
    cpu.step(mem);
    CHECK_HEX(T::a(cpu), 0xB5);
    CHECK_FLAGS(cpu, 0);

    // RLC, RRC, RAL, RAR, STC, CMC leave S, Z, AC, P unchanged
    for (int opcode : {0x07, 0x0F, 0x17, 0x1F, 0x37, 0x3F}) {
        INFO("opcode " << Hex(opcode));
        cpu = make_cpu({(uint8_t)opcode});
        T::a(cpu) = 0x00;
        T::flags(cpu) = S | Z | AC | P;
        cpu.step(mem);
        CHECK_HEX(T::flags(cpu) & (S | Z | AC | P), S | Z | AC | P);
    }
}

TEST_CASE("CMA STC CMC", "[alu]") {
    CPU cpu = make_cpu({0x2F});            // CMA
    T::a(cpu) = 0x51;
    T::flags(cpu) = Z;
    CHECK(cpu.step(mem) == 4);
    CHECK_HEX(T::a(cpu), 0xAE);
    CHECK_FLAGS(cpu, Z);

    cpu = make_cpu({0x37, 0x3F, 0x3F});    // STC; CMC; CMC
    CHECK(cpu.step(mem) == 4);
    CHECK_FLAGS(cpu, CY);
    CHECK(cpu.step(mem) == 4);
    CHECK_FLAGS(cpu, 0);
    cpu.step(mem);
    CHECK_FLAGS(cpu, CY);
}


// --- Cycle counts ---

TEST_CASE("ALU cycle counts", "[alu]") {
    // 0x80 - 0xBF: M form 7 cycles, register forms 4
    for (int opcode = 0x80; opcode <= 0xBF; opcode++) {
        INFO("opcode " << Hex(opcode));
        CPU cpu = make_cpu({(uint8_t)opcode});
        T::h(cpu) = 0x20;
        T::l(cpu) = 0x00;
        CHECK(cpu.step(mem) == ((opcode & 0x07) == 6 ? 7u : 4u));
    }
}


// --- Flag byte layout ---

TEST_CASE("Unused flag bits preserved", "[alu]") {
    // Bits 1, 3, 5 unchanged
    CPU cpu = make_cpu({0x80});            // ADD B
    T::flags(cpu) = 0x02;
    T::a(cpu) = 0x01;
    T::b(cpu) = 0x01;
    cpu.step(mem);
    CHECK_HEX(T::flags(cpu) & 0x2A, 0x02);
}

