#ifndef INVADERS_INTERRUPT_SCHEDULE_HPP
#define INVADERS_INTERRUPT_SCHEDULE_HPP

#include <cstdint>
#include <optional>

// Handles frame count and raises interrupts:
// RST 1 at mid-frame and RST 2 at end of frame (vblank).

class InterruptSchedule {
public:
    // Advances the frame by n cycles. Returns true when a frame completes
    bool add_cycles(uint32_t cycles);

    bool has_pending() const { return pending_rst_.has_value(); }
    void clear_pending() { pending_rst_.reset(); }

    // The RST number (1 or 2) waiting to be delivered, or nullopt if none
    std::optional<uint8_t> pending_rst() const { return pending_rst_; }

    uint32_t frame_cycles() const { return frame_cycles_; }

private:
    uint32_t frame_cycles_ = 0;
    bool mid_screen_raised_ = false;
    std::optional<uint8_t> pending_rst_;
};

#endif
