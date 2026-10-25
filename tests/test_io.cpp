// Unit tests for IN and OUT instructions.

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "cpu_test_helpers.hpp"
#include "../src/core/io_bus.hpp"

// Test bus: returns preset values for IN and records every IN/OUT
struct FakeIO : IOBus {
    std::array<uint8_t, 256> inputs{};
    std::vector<uint8_t> ports_read;
    std::vector<std::pair<uint8_t, uint8_t>> writes;    // (port, value)

    uint8_t in(uint8_t port) override {
        ports_read.push_back(port);
        return inputs[port];
    }

    void out(uint8_t port, uint8_t value) override {
        writes.emplace_back(port, value);
    }
};

using Write = std::pair<uint8_t, uint8_t>;


// --- IN ---

TEST_CASE("IN loads A from the operand port", "[cpu][io]") {
    FakeIO io;
    io.inputs[0x01] = 0x5A;
    CPU cpu = make_cpu({0xDB, 0x01});          // IN 0x01
    cpu.set_io(&io);

    CHECK(cpu.step(mem) == 10);
    CHECK(T::a(cpu) == 0x5A);
    CHECK(T::pc(cpu) == ORIGIN + 2);
    CHECK(io.ports_read == std::vector<uint8_t>{0x01});
    CHECK(io.writes.empty());
}

TEST_CASE("IN reads the port named by the operand", "[cpu][io]") {
    uint8_t port = GENERATE(0x00, 0x02, 0x03, 0x80, 0xFF);
    CAPTURE((int)port);

    FakeIO io;
    io.inputs[port] = port ^ 0xA5;             // distinct value per port
    CPU cpu = make_cpu({0xDB, port});
    cpu.set_io(&io);

    cpu.step(mem);
    CHECK(T::a(cpu) == (port ^ 0xA5));
    CHECK(io.ports_read == std::vector<uint8_t>{port});
}

TEST_CASE("IN does not change flags or other registers", "[cpu][io]") {
    FakeIO io;
    io.inputs[0x02] = 0x00;                    // a zero result must not set Z
    CPU cpu = make_cpu({0xDB, 0x02}, ALL_FLAGS);
    cpu.set_io(&io);
    T::b(cpu) = 0x11;
    T::h(cpu) = 0x22;
    T::sp(cpu) = 0x2400;

    cpu.step(mem);
    CHECK(T::a(cpu) == 0x00);
    CHECK(T::flags(cpu) == ALL_FLAGS);
    CHECK(T::b(cpu) == 0x11);
    CHECK(T::h(cpu) == 0x22);
    CHECK(T::sp(cpu) == 0x2400);
}


// --- OUT ---

TEST_CASE("OUT sends A to the operand port", "[cpu][io]") {
    FakeIO io;
    CPU cpu = make_cpu({0xD3, 0x03});          // OUT 0x03
    cpu.set_io(&io);
    T::a(cpu) = 0x77;

    CHECK(cpu.step(mem) == 10);
    CHECK(io.writes == std::vector<Write>{{0x03, 0x77}});
    CHECK(T::a(cpu) == 0x77);
    CHECK(T::pc(cpu) == ORIGIN + 2);
    CHECK(io.ports_read.empty());
}

TEST_CASE("OUT does not change flags", "[cpu][io]") {
    FakeIO io;
    CPU cpu = make_cpu({0xD3, 0x05}, ALL_FLAGS);
    cpu.set_io(&io);

    cpu.step(mem);
    CHECK(io.writes == std::vector<Write>{{0x05, 0x00}});    // A = 0 is still written
    CHECK(T::flags(cpu) == ALL_FLAGS);
}

TEST_CASE("Multiple OUTs reach the bus in program order", "[cpu][io]") {
    FakeIO io;
    CPU cpu = make_cpu({
        0x3E, 0x12,                            // MVI A, 0x12
        0xD3, 0x04,                            // OUT 4
        0x3E, 0x34,                            // MVI A, 0x34
        0xD3, 0x04,                            // OUT 4
        0xD3, 0x02,                            // OUT 2
    });
    cpu.set_io(&io);

    for (int i = 0; i < 5; i++)
        cpu.step(mem);

    CHECK(io.writes == std::vector<Write>{{0x04, 0x12}, {0x04, 0x34}, {0x02, 0x34}});
}


// --- No bus attached ---

TEST_CASE("IN with no bus attached reads 0xFF", "[cpu][io]") {
    CPU cpu = make_cpu({0xDB, 0x01});

    CHECK(cpu.step(mem) == 10);
    CHECK(T::a(cpu) == 0xFF);
    CHECK(T::pc(cpu) == ORIGIN + 2);
}

TEST_CASE("OUT with no bus attached is ignored", "[cpu][io]") {
    CPU cpu = make_cpu({0xD3, 0x03});
    T::a(cpu) = 0x77;

    CHECK(cpu.step(mem) == 10);
    CHECK(T::a(cpu) == 0x77);
    CHECK(T::pc(cpu) == ORIGIN + 2);
}
