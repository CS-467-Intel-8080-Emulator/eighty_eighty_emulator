#ifndef INVADERS_INTERRUPT_SCHEDULE_HPP
#define INVADERS_INTERRUPT_SCHEDULE_HPP

#include <cstdint>

// Handles frame count and raises interrupts: 
// RST 1 at mid-frame and RST 2 at end of frame (vblank). 

class InterruptSchedule {
public:
    bool add_cycles(uint32_t cycles);
    bool has_pending() const { return pending_rst_ != NONE; }
    void clear_pending() { pending_rst_ = NONE; }
    uint8_t pending_rst() const { return pending_rst_; }
    uint32_t frame_cycles() const { return frame_cycles_; }

private:
    static constexpr uint8_t NONE = 0xFF;
    uint32_t frame_cycles_ = 0;
    bool mid_screen_raised_ = false;
    uint8_t pending_rst_ = NONE;
};

#endif
