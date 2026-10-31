#include "interrupt_schedule.hpp"
#include "timing.hpp"

bool InterruptSchedule::add_cycles(uint32_t cycles) {
    frame_cycles_ += cycles;

    if (!mid_screen_raised_ && frame_cycles_ >= invaders::MID_SCREEN_CYCLE) {
        pending_rst_ = 1;
        mid_screen_raised_ = true;
    }

    if (frame_cycles_ < invaders::CYCLES_PER_FRAME)
        return false;

    // Catches frame overshoot that frame_cycles = 0 would miss
    frame_cycles_ -= invaders::CYCLES_PER_FRAME;
    mid_screen_raised_ = false;

    // Replaces an undelivered RST 1: the bus carries the current beam position
    pending_rst_ = 2;
    return true;
}
