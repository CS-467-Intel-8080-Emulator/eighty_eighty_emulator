#ifndef INVADERS_MACHINE_HPP
#define INVADERS_MACHINE_HPP

#include <cstdint>
#include <vector>

#include "core/cpu.hpp"
#include "core/flat_memory.hpp"
#include "platform/machine.hpp"

#include "video.hpp"
#include "interrupt_schedule.hpp"
#include "invaders_io.hpp"
#include "timing.hpp"

class InvadersMachine : public Machine {
public:
    InvadersMachine();
    InvadersMachine(const InvadersMachine&) = delete; // Disable creating new machine as copy
    InvadersMachine& operator=(const InvadersMachine&) = delete; // Disable overwriting machine with copy

    const char* title() const override { return "Space Invaders"; }
    int display_width() const override { return Video::SCREEN_WIDTH; }
    int display_height() const override { return Video::SCREEN_HEIGHT; }
    double frames_per_second() const override { return invaders::FRAMES_PER_SECOND; }

    void load_rom(uint16_t address, const std::vector<uint8_t>& bytes); // TODO: ROM loading
    void run_frame() override;
    const uint32_t* pixels() const override { return video_.framebuffer().data(); }

private:
    CPU cpu_;
    FlatMemory memory_;  // TODO: replace with the Space Invaders memory
    InvadersIO io_;
    InterruptSchedule schedule_;
    Video video_;
};

#endif
