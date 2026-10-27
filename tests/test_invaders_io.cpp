// Unit tests for the Space Invaders port map, alone and driven by the CPU.

#include <cstdint>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "cpu_test_helpers.hpp"
#include "../src/platform/invaders/invaders_io.hpp"


// --- Port routing ---

TEST_CASE("OUT 4, OUT 2, and IN 3 reach the shift register", "[invaders][io]") {
    InvadersIO io;
    io.out(4, 0x12);
    io.out(4, 0x34);                        // register = 0x3412
    io.out(2, 4);
    CHECK(io.in(3) == 0x41);
}

TEST_CASE("Sound and watchdog writes leave the shift register alone", "[invaders][io]") {
    uint8_t port = GENERATE(3, 5, 6);
    CAPTURE((int)port);

    InvadersIO io;
    io.out(4, 0x12);
    io.out(4, 0x34);
    io.out(port, 0xFF);
    CHECK(io.in(3) == 0x34);
}

TEST_CASE("Ports without a device read 0x00", "[invaders][io]") {
    uint8_t port = GENERATE(0x04, 0x07, 0xFF);
    CAPTURE((int)port);

    InvadersIO io;
    CHECK(io.in(port) == 0x00);
}


// --- Driven by the CPU ---

TEST_CASE("CPU program shifts a value through the port map", "[invaders][io][cpu]") {
    InvadersIO io;
    CPU cpu = make_cpu({
        0x3E, 0x12,                         // MVI A, 0x12
        0xD3, 0x04,                         // OUT 4
        0x3E, 0x34,                         // MVI A, 0x34
        0xD3, 0x04,                         // OUT 4
        0x3E, 0x04,                         // MVI A, 4
        0xD3, 0x02,                         // OUT 2
        0xDB, 0x03,                         // IN 3
    });
    cpu.set_io(&io);

    for (int i = 0; i < 7; i++)
        cpu.step(mem);

    CHECK(T::a(cpu) == 0x41);
    CHECK(T::pc(cpu) == ORIGIN + 14);
}
