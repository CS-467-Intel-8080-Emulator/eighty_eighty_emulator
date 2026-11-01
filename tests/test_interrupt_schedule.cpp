// Unit tests for the Space Invaders timing constants and interrupt schedule.

#include <catch2/catch_test_macros.hpp>

#include "../src/platform/invaders/interrupt_schedule.hpp"
#include "../src/platform/invaders/timing.hpp"

using namespace invaders;


// --- Timing constants ---

TEST_CASE("Timing constants match the hardware", "[invaders][timing]") {
    STATIC_REQUIRE(CYCLES_PER_LINE == 128);
    STATIC_REQUIRE(CYCLES_PER_FRAME == 33'536);
    STATIC_REQUIRE(MID_SCREEN_CYCLE == 17'152);
}


// --- Interrupt schedule ---

TEST_CASE("No interrupt before mid-screen", "[invaders][interrupts]") {
    InterruptSchedule schedule;
    CHECK_FALSE(schedule.add_cycles(MID_SCREEN_CYCLE - 1));
    CHECK_FALSE(schedule.has_pending());
}

TEST_CASE("RST 1 is raised at mid-screen", "[invaders][interrupts]") {
    InterruptSchedule schedule;
    CHECK_FALSE(schedule.add_cycles(MID_SCREEN_CYCLE));
    CHECK(schedule.has_pending());
    CHECK(schedule.pending_rst() == 1);
}

TEST_CASE("RST 1 is raised once per frame", "[invaders][interrupts]") {
    InterruptSchedule schedule;
    schedule.add_cycles(MID_SCREEN_CYCLE);
    schedule.clear_pending();

    schedule.add_cycles(100);
    CHECK_FALSE(schedule.has_pending());
}

TEST_CASE("RST 2 is raised and the frame ends at vblank", "[invaders][interrupts]") {
    InterruptSchedule schedule;
    schedule.add_cycles(MID_SCREEN_CYCLE);
    schedule.clear_pending();

    CHECK(schedule.add_cycles(CYCLES_PER_FRAME - MID_SCREEN_CYCLE));
    CHECK(schedule.pending_rst() == 2);
    CHECK(schedule.frame_cycles() == 0);
}

TEST_CASE("Overshoot carries into the next frame", "[invaders][interrupts]") {
    InterruptSchedule schedule;
    schedule.add_cycles(CYCLES_PER_FRAME - 2);
    CHECK(schedule.add_cycles(10));
    CHECK(schedule.frame_cycles() == 8);
}

TEST_CASE("A refused interrupt stays pending", "[invaders][interrupts]") {
    InterruptSchedule schedule;
    schedule.add_cycles(MID_SCREEN_CYCLE);
    schedule.add_cycles(1000);                 // CPU had interrupts disabled
    CHECK(schedule.pending_rst() == 1);
}

TEST_CASE("Undelivered RST 1 is replaced by RST 2", "[invaders][interrupts]") {
    InterruptSchedule schedule;
    schedule.add_cycles(MID_SCREEN_CYCLE);
    schedule.add_cycles(CYCLES_PER_FRAME - MID_SCREEN_CYCLE);
    CHECK(schedule.pending_rst() == 2);
}
