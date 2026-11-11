#include "interrupt_schedule.hpp"
#include "timing.hpp"
#include "tools/state.hpp"

bool InterruptSchedule::add_cycles(uint32_t cycles) {
    frame_cycles_ += cycles;

    if (!mid_screen_raised_ && frame_cycles_ >= invaders::MID_SCREEN_CYCLE) {
        pending_rst_ = 1;
        mid_screen_raised_ = true;
    }
    // Frame hasn't finished yet, so stop early 
    if (frame_cycles_ < invaders::CYCLES_PER_FRAME)
        return false;

    // Catches frame overshoot that frame_cycles = 0 would miss
    frame_cycles_ -= invaders::CYCLES_PER_FRAME;
    mid_screen_raised_ = false;

    // Replaces an undelivered RST 1: the bus carries the current beam position
    pending_rst_ = 2;
    return true;
}

void InterruptSchedule::save_state(StateWriter& writer) const {
    writer.save_u32(frame_cycles_);
    writer.save_u8(static_cast<uint8_t>(mid_screen_raised_));
    writer.save_u8(pending_rst_.value_or(0)); // RST, if pending, will only ever be 1 or 2; 0 if none pending
}

void InterruptSchedule::load_state(StateReader& reader) {
    frame_cycles_ = reader.load_u32();
    mid_screen_raised_ = static_cast<bool>(reader.load_u8());
    if (uint8_t rst = reader.load_u8()) {
        pending_rst_ = rst;
    } else {
        pending_rst_.reset();
    }
}
