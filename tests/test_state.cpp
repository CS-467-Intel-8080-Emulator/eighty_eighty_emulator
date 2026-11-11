// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcript documents the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/14UFLJnbF9rFUq6pEIBuArmQJAz6fYccr?usp=drive_link

// Unit tests for save and load state: the byte writer/reader, each component,
// and the full Invaders machine.

#include <cstdint>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "cpu_test_helpers.hpp"
#include "../src/state/state.hpp"
#include "../src/platform/invaders/interrupt_schedule.hpp"
#include "../src/platform/invaders/invaders_io.hpp"
#include "../src/platform/invaders/invaders_machine.hpp"
#include "../src/platform/invaders/invaders_memory.hpp"
#include "../src/platform/invaders/shift_register.hpp"
#include "../src/platform/invaders/timing.hpp"

using Bytes = std::vector<uint8_t>;

// Saves a component into a fresh buffer
template <typename Component>
Bytes save(const Component& component) {
    StateWriter writer;
    component.save_state(writer);
    return writer.get_buffer();
}

// Loads a buffer into a component
template <typename Component>
void load(Component& component, const Bytes& bytes) {
    StateReader reader;
    reader.set_buffer(bytes);
    component.load_state(reader);
}


// --- StateWriter and StateReader ---

TEST_CASE("Writer stores values low byte first", "[state]") {
    StateWriter writer;
    writer.save_u8(0x12);
    writer.save_u16(0x3456);
    writer.save_u32(0x789ABCDE);
    CHECK(writer.get_buffer() == Bytes{0x12, 0x56, 0x34, 0xDE, 0xBC, 0x9A, 0x78});
}

TEST_CASE("Reader reads back what the writer wrote", "[state]") {
    StateWriter writer;
    writer.save_u8(0xFF);
    writer.save_u16(0x8001);
    writer.save_u32(0xF0000001);   // Top bit set: checks the u32 shift

    StateReader reader;
    reader.set_buffer(writer.get_buffer());
    CHECK(reader.load_u8() == 0xFF);
    CHECK(reader.load_u16() == 0x8001);
    CHECK(reader.load_u32() == 0xF0000001);
}

TEST_CASE("Reader throws when reading past the end", "[state]") {
    StateReader reader;
    reader.set_buffer({0x01});
    reader.load_u8();
    CHECK_THROWS_AS(reader.load_u8(), std::out_of_range);
}

TEST_CASE("set_buffer restarts reading from the beginning", "[state]") {
    StateReader reader;
    reader.set_buffer({0x01, 0x02});
    reader.load_u8();
    reader.set_buffer({0xAA});
    CHECK(reader.load_u8() == 0xAA);
}


// --- CPU ---

TEST_CASE("CPU state round-trips every register and flag", "[state][cpu]") {
    CPU original;
    T::a(original) = 0x11;
    T::b(original) = 0x22;
    T::c(original) = 0x33;
    T::d(original) = 0x44;
    T::e(original) = 0x55;
    T::h(original) = 0x66;
    T::l(original) = 0x77;
    T::pc(original) = 0x1234;
    T::sp(original) = 0x2345;
    T::flags(original) = 0xD7;
    T::interrupts_enabled(original) = true;
    T::ei_pending(original) = true;
    T::halted(original) = true;

    Bytes bytes = save(original);
    CHECK(bytes.size() == 15);

    CPU copy;
    load(copy, bytes);
    CHECK(T::a(copy) == 0x11);
    CHECK(T::b(copy) == 0x22);
    CHECK(T::c(copy) == 0x33);
    CHECK(T::d(copy) == 0x44);
    CHECK(T::e(copy) == 0x55);
    CHECK(T::h(copy) == 0x66);
    CHECK(T::l(copy) == 0x77);
    CHECK(T::pc(copy) == 0x1234);
    CHECK(T::sp(copy) == 0x2345);
    CHECK(T::flags(copy) == 0xD7);
    CHECK(T::interrupts_enabled(copy));
    CHECK(T::ei_pending(copy));
    CHECK(T::halted(copy));
}

TEST_CASE("CPU load clears flags that were set before", "[state][cpu]") {
    CPU target;
    T::interrupts_enabled(target) = true;
    T::ei_pending(target) = true;
    T::halted(target) = true;

    load(target, save(CPU()));   // Fresh CPU: all three flags false
    CHECK_FALSE(T::interrupts_enabled(target));
    CHECK_FALSE(T::ei_pending(target));
    CHECK_FALSE(T::halted(target));
}


// --- Shift register and IO ---

TEST_CASE("Shift register state round-trips value and offset", "[state][invaders]") {
    ShiftRegister original;
    original.write_data(0xAB);
    original.write_data(0xCD);   // register = 0xCDAB
    original.write_offset(3);

    Bytes bytes = save(original);
    CHECK(bytes == Bytes{0xAB, 0xCD, 0x03});

    ShiftRegister copy;
    load(copy, bytes);
    CHECK(copy.read() == original.read());
}

TEST_CASE("InvadersIO state carries the shift register", "[state][invaders][io]") {
    InvadersIO original;
    original.out(4, 0x12);
    original.out(4, 0x34);   // register = 0x3412
    original.out(2, 4);

    InvadersIO copy;
    load(copy, save(original));
    CHECK(copy.in(3) == 0x41);
}


// --- Interrupt schedule ---

TEST_CASE("Schedule state round-trips a pending RST 1", "[state][invaders][interrupts]") {
    InterruptSchedule original;
    original.add_cycles(invaders::MID_SCREEN_CYCLE + 10);   // Raises RST 1

    Bytes bytes = save(original);
    CHECK(bytes.size() == 6);

    InterruptSchedule copy;
    load(copy, bytes);
    CHECK(copy.frame_cycles() == original.frame_cycles());
    CHECK(copy.pending_rst() == 1);
}

TEST_CASE("Schedule state round-trips no pending interrupt", "[state][invaders][interrupts]") {
    InterruptSchedule original;
    original.add_cycles(invaders::MID_SCREEN_CYCLE + 10);
    original.clear_pending();

    InterruptSchedule copy;
    copy.add_cycles(invaders::MID_SCREEN_CYCLE);   // Copy has RST 1 pending before the load
    load(copy, save(original));
    CHECK_FALSE(copy.has_pending());
}

TEST_CASE("Loaded schedule does not raise RST 1 twice in a frame", "[state][invaders][interrupts]") {
    // If mid_screen_raised_ weren't restored, the copy would raise RST 1 again
    InterruptSchedule original;
    original.add_cycles(invaders::MID_SCREEN_CYCLE + 10);
    original.clear_pending();

    InterruptSchedule copy;
    load(copy, save(original));
    copy.add_cycles(10);
    CHECK_FALSE(copy.has_pending());
}


// --- Memory ---

TEST_CASE("Memory state saves RAM and VRAM but not ROM", "[state][invaders][memory]") {
    InvadersMemory original;
    original.load_rom({0xAA, 0xBB});
    original.write(0x2000, 0x01);   // First byte of work RAM
    original.write(0x2400, 0x02);   // First byte of VRAM
    original.write(0x3FFF, 0x03);   // Last byte of VRAM

    Bytes bytes = save(original);
    CHECK(bytes.size() == 0x2000);

    InvadersMemory copy;
    copy.load_rom({0x11, 0x22});    // Different ROM
    load(copy, bytes);
    CHECK(copy.read(0x2000) == 0x01);
    CHECK(copy.read(0x2400) == 0x02);
    CHECK(copy.read(0x3FFF) == 0x03);
    CHECK(copy.read(0x0000) == 0x11);   // ROM untouched by the load
    CHECK(copy.read(0x0001) == 0x22);
}


// --- Full machine ---

// Small program that keeps changing registers, RAM, and the stack, and
// handles both interrupts, so the machine has non-trivial state to save
static const Bytes TEST_ROM = [] {
    Bytes rom(0x30, 0x00);
    auto put = [&](uint16_t addr, Bytes code) {
        for (uint8_t byte : code) rom[addr++] = byte;
    };
    put(0x0000, {0xC3, 0x20, 0x00});          // JMP 0x0020
    put(0x0008, {0x14, 0xFB, 0xC9});          // RST 1: INR D; EI; RET
    put(0x0010, {0x1C, 0xFB, 0xC9});          // RST 2: INR E; EI; RET
    put(0x0020, {0x31, 0x00, 0x24,            // LXI SP, 0x2400
                 0xFB,                        // EI
                 0x3C,                        // loop: INR A
                 0x32, 0x00, 0x21,            //       STA 0x2100
                 0x03,                        //       INX B
                 0xC3, 0x24, 0x00});          //       JMP loop
    return rom;
}();

TEST_CASE("Machine save is the expected size", "[state][invaders][machine]") {
    InvadersMachine machine;
    CHECK(machine.save_state().size() == 8216);
}

TEST_CASE("Machine load restores the exact saved state", "[state][invaders][machine]") {
    InvadersMachine original;
    REQUIRE(original.load_rom(TEST_ROM));
    for (int i = 0; i < 3; i++) original.run_frame();
    Bytes saved = original.save_state();

    InvadersMachine copy;
    REQUIRE(copy.load_rom(TEST_ROM));
    REQUIRE(copy.load_state(saved));
    CHECK(copy.save_state() == saved);
}

TEST_CASE("Loaded machine runs the same as the original", "[state][invaders][machine]") {
    InvadersMachine original;
    REQUIRE(original.load_rom(TEST_ROM));
    for (int i = 0; i < 3; i++) original.run_frame();
    Bytes saved = original.save_state();

    InvadersMachine copy;
    REQUIRE(copy.load_rom(TEST_ROM));
    REQUIRE(copy.load_state(saved));

    for (int i = 0; i < 5; i++) {
        original.run_frame();
        copy.run_frame();
    }
    CHECK(copy.save_state() == original.save_state());
    CHECK(copy.save_state() != saved);   // Sanity check: the program changed state
}

TEST_CASE("Machine rejects a buffer of the wrong size and keeps its state", "[state][invaders][machine]") {
    InvadersMachine machine;
    REQUIRE(machine.load_rom(TEST_ROM));
    machine.run_frame();
    Bytes before = machine.save_state();

    Bytes too_short(100, 0xFF);
    CHECK_FALSE(machine.load_state(too_short));

    Bytes too_long(before.size() + 1, 0xFF);
    CHECK_FALSE(machine.load_state(too_long));

    CHECK(machine.save_state() == before);
}
