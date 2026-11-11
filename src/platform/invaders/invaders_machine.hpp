// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcripts document the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/1ju4Le0a1KcFW42RTQtTtEUDd8LGAbAAA?usp=drive_link

#ifndef INVADERS_MACHINE_HPP
#define INVADERS_MACHINE_HPP

#include <cstdint>
#include <vector>

#include "core/cpu.hpp"
#include "platform/machine.hpp"

#include "invaders_memory.hpp"
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

    bool load_rom(const std::vector<uint8_t>& bytes, uint16_t offset = 0);
    void run_frame() override;
    const uint32_t* pixels() const override { return video_.framebuffer().data(); }

    // Save and load machine state
    std::vector<uint8_t> save_state() const override;
    bool load_state(const std::vector<uint8_t>& buffer) override;

private:
    CPU cpu_;
    InvadersMemory memory_;
    InvadersIO io_;
    InterruptSchedule schedule_;
    Video video_;

    static constexpr std::size_t STATE_SIZE = 8216; // Total size of serialized Invaders machine state in bytes
};

#endif
