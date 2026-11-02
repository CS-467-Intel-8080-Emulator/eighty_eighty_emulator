// Unit tests for the Space Invaders shift register.

#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "../src/platform/invaders/shift_register.hpp"

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


// --- Data writes ---

TEST_CASE("Shift register starts at zero", "[invaders][shift]") {
    ShiftRegister shift;
    CHECK_HEX(shift.read(), 0x00);
}

TEST_CASE("Data write becomes the high byte", "[invaders][shift]") {
    ShiftRegister shift;
    shift.write_data(0xAB);
    CHECK_HEX(shift.read(), 0xAB);
}

TEST_CASE("Each data write replaces the high byte", "[invaders][shift]") {
    ShiftRegister shift;
    shift.write_data(0x12);
    shift.write_data(0x34);
    shift.write_data(0x56);
    CHECK_HEX(shift.read(), 0x56);
}

TEST_CASE("Reading does not change the register", "[invaders][shift]") {
    ShiftRegister shift;
    shift.write_data(0x5A);
    CHECK_HEX(shift.read(), 0x5A);
    CHECK_HEX(shift.read(), 0x5A);
}


// --- Offset ---

// Register holding high:low with the offset set
static ShiftRegister loaded(uint8_t high, uint8_t low, uint8_t offset) {
    ShiftRegister shift;
    shift.write_data(low);
    shift.write_data(high);
    shift.write_offset(offset);
    return shift;
}

struct OffsetCase { uint8_t offset; uint8_t expected; };

TEST_CASE("Offset selects which 8 bits are read", "[invaders][shift]") {
    SECTION("0xF00F") {
        OffsetCase c = GENERATE(
            OffsetCase{0, 0xF0},
            OffsetCase{1, 0xE0},
            OffsetCase{2, 0xC0},
            OffsetCase{3, 0x80},
            OffsetCase{4, 0x00},
            OffsetCase{5, 0x01},
            OffsetCase{6, 0x03},
            OffsetCase{7, 0x07}
        );
        CAPTURE((int)c.offset);
        CHECK_HEX(loaded(0xF0, 0x0F, c.offset).read(), c.expected);
    }

    SECTION("0x3412") {
        // Uneven value catches swapped or rotated bytes
        OffsetCase c = GENERATE(
            OffsetCase{0, 0x34},
            OffsetCase{1, 0x68},
            OffsetCase{2, 0xD0},
            OffsetCase{3, 0xA0},
            OffsetCase{4, 0x41},
            OffsetCase{5, 0x82},
            OffsetCase{6, 0x04},
            OffsetCase{7, 0x09}
        );
        CAPTURE((int)c.offset);
        CHECK_HEX(loaded(0x34, 0x12, c.offset).read(), c.expected);
    }
}

TEST_CASE("Offset uses only the low 3 bits", "[invaders][shift]") {
    OffsetCase c = GENERATE(
        OffsetCase{0x08, 0xF0},             // acts as 0
        OffsetCase{0x0B, 0x80},             // acts as 3
        OffsetCase{0xF9, 0xE0},             // acts as 1
        OffsetCase{0xFF, 0x07}              // acts as 7
    );
    CAPTURE((int)c.offset);
    CHECK_HEX(loaded(0xF0, 0x0F, c.offset).read(), c.expected);
}

TEST_CASE("Offset stays set across data writes", "[invaders][shift]") {
    ShiftRegister shift;
    shift.write_offset(4);
    shift.write_data(0xAB);                 // 0xAB00 -> 0xB0
    CHECK_HEX(shift.read(), 0xB0);
    shift.write_data(0xCD);                 // 0xCDAB -> 0xDA
    CHECK_HEX(shift.read(), 0xDA);
    shift.write_data(0xEF);                 // 0xEFCD -> 0xFC, 0xAB dropped
    CHECK_HEX(shift.read(), 0xFC);
}
