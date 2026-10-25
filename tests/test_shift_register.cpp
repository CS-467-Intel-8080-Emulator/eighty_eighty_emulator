// Unit tests for the Space Invaders shift register.

#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>

#include <catch2/catch_test_macros.hpp>

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
