// Unit tests for the CP/M test machine (CpmMachine), using tiny hand-assembled programs.
// Programs load at 0x0100, so a byte's address is 0x0100 + its index in the vector.

#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "platform/cpm_test/cpm_machine.hpp"

// Prints "HELLO" with BDOS function 9, then "!" with function 2, then exits.
static const std::vector<uint8_t> HELLO = {
    0x0E, 0x09,         // 0100  MVI C,9       print string
    0x16, 0x01,         // 0102  MVI D,01h   ┐ DE = 0113h, the text below
    0x1E, 0x13,         // 0104  MVI E,13h   ┘
    0xCD, 0x05, 0x00,   // 0106  CALL 0005h
    0x0E, 0x02,         // 0109  MVI C,2       print character
    0x1E, '!',          // 010B  MVI E,'!'
    0xCD, 0x05, 0x00,   // 010D  CALL 0005h
    0xC3, 0x00, 0x00,   // 0110  JMP 0000h     exit
    'H', 'E', 'L', 'L', 'O', '$',   // 0113
};


// --- load() ---

TEST_CASE("A program that fills memory up to 0xFFFF loads", "[cpm]") {
    CpmMachine machine;
    CHECK(machine.load(std::vector<uint8_t>(0x10000 - 0x0100)));
}

TEST_CASE("A program too big to fit is rejected", "[cpm]") {
    CpmMachine machine;
    CHECK_FALSE(machine.load(std::vector<uint8_t>(0x10000 - 0x0100 + 1)));
}


// --- run(): how a program ends ---

TEST_CASE("Jumping to 0x0000 finishes the program", "[cpm]") {
    CpmMachine machine;
    machine.load({0xC3, 0x00, 0x00});   // JMP 0000h
    CHECK(machine.run());
    CHECK(machine.get_cycles() == 10);
}

TEST_CASE("A halted CPU stops the program early", "[cpm]") {
    CpmMachine machine;
    machine.load({0x76});               // HLT
    CHECK_FALSE(machine.run());
}

TEST_CASE("Exceeding the cycle limit stops the program early", "[cpm]") {
    CpmMachine machine;
    machine.load({0xC3, 0x00, 0x01});   // JMP 0100h: loops forever
    CHECK_FALSE(machine.run(1000));
    CHECK(machine.get_cycles() >= 1000);
    CHECK(machine.get_cycles() < 1000 + 10);   // stops within one JMP of the limit
}


// --- BDOS calls ---

TEST_CASE("BDOS function 9 prints text up to '$'", "[cpm][bdos]") {
    CpmMachine machine;
    machine.load({
        0x0E, 0x09,         // 0100  MVI C,9
        0x16, 0x01,         // 0102  MVI D,01h
        0x1E, 0x0C,         // 0104  MVI E,0Ch   DE = 010Ch
        0xCD, 0x05, 0x00,   // 0106  CALL 0005h
        0xC3, 0x00, 0x00,   // 0109  JMP 0000h
        'H', 'I', '$', 'X', // 010C  'X' is after the '$', so it must not print
    });
    CHECK(machine.run());
    CHECK(machine.get_output() == "HI");
}

TEST_CASE("BDOS function 2 prints the character in E", "[cpm][bdos]") {
    CpmMachine machine;
    machine.load({
        0x0E, 0x02,         // MVI C,2
        0x1E, '!',          // MVI E,'!'
        0xCD, 0x05, 0x00,   // CALL 0005h
        0xC3, 0x00, 0x00,   // JMP 0000h
    });
    CHECK(machine.run());
    CHECK(machine.get_output() == "!");
}

TEST_CASE("A BDOS call returns to the instruction after the CALL", "[cpm][bdos]") {
    // HELLO's second print only happens if the first call returned correctly
    CpmMachine machine;
    machine.load(HELLO);
    CHECK(machine.run());
    CHECK(machine.get_output() == "HELLO!");
    CHECK(machine.get_cycles() == 79);   // 5 MVI x 7 + 2 CALL x 17 + 1 JMP x 10
}

TEST_CASE("Function 9 stops after one pass through memory if there's no '$'", "[cpm][bdos]") {
    // None of these bytes, the top-of-memory bytes, or the return address is '$' (0x24)
    CpmMachine machine;
    machine.load({
        0x0E, 0x09,         // MVI C,9
        0x16, 0x80,         // MVI D,80h
        0x1E, 0x00,         // MVI E,00h   DE = 8000h
        0xCD, 0x05, 0x00,   // CALL 0005h
        0xC3, 0x00, 0x00,   // JMP 0000h
    });
    CHECK(machine.run());
    CHECK(machine.get_output().size() == 0x10000);
}

TEST_CASE("An unimplemented BDOS function reports an error", "[cpm][bdos]") {
    CpmMachine machine;
    machine.load({
        0x0E, 0x01,         // MVI C,1     console input: not implemented
        0xCD, 0x05, 0x00,   // CALL 0005h
        0xC3, 0x00, 0x00,   // JMP 0000h
    });
    machine.run();
    CHECK(machine.get_output().find("ERROR: Unimplemented BDOS function 1") != std::string::npos);
}


// --- Reuse ---

TEST_CASE("load() resets the machine for a new program", "[cpm]") {
    CpmMachine machine;
    machine.load({0x76});               // HLT: leaves the CPU halted
    machine.run();

    machine.load(HELLO);
    CHECK(machine.run());               // would stop immediately if still halted
    CHECK(machine.get_output() == "HELLO!");
    CHECK(machine.get_cycles() == 79);  // not added to the first run's cycles
}
