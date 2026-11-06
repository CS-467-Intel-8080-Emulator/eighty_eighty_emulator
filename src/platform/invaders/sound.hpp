#ifndef INVADERS_SOUND_HPP
#define INVADERS_SOUND_HPP

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

// Space Invaders sound board. Each bit of OUT 3 and OUT 5 turns on one sound:
//   OUT 3: 0 UFO, 1 shot, 2 player death, 3 invader death, 4 extra life, 5 amp enable
//   OUT 5: 0-3 fleet movement, 4 UFO hit
// The UFO loops while its bit is set. The others play once when their bit goes
// from 0 to 1. A sound with no sample is silent.
//
// write_port3/write_port5 run on the emulator thread and render on the audio
// thread. They share only atomics.
class InvadersSound {
public:
    static constexpr int SAMPLE_RATE = 48000;

    // Numbered to match 0.wav-9.wav
    enum Sound { UFO, SHOT, PLAYER_DEATH, INVADER_DEATH,
                 FLEET_1, FLEET_2, FLEET_3, FLEET_4, UFO_HIT, EXTRA_LIFE, SOUND_COUNT };

    // Mono samples at SAMPLE_RATE. Call before rendering starts.
    void set_sample(Sound sound, std::vector<float> samples);

    // Emulator thread
    void write_port3(uint8_t value);
    void write_port5(uint8_t value);

    // Audio thread
    void render(float* out, int count);

private:
    // Port 3 bits
    static constexpr uint8_t BIT_UFO = 0x01;
    static constexpr uint8_t BIT_AMP_ENABLE = 0x20;

    // Sound for each bit, -1 for none
    static const int8_t PORT3_SOUNDS[8];
    static const int8_t PORT5_SOUNDS[8];

    void count_rising_edges(uint8_t old_value, uint8_t new_value, const int8_t (&sounds)[8]);

    std::array<std::vector<float>, SOUND_COUNT> samples_{};

    // Shared between threads
    std::atomic<uint8_t> port3_{ 0 };
    std::atomic<uint8_t> port5_{ 0 };
    std::array<std::atomic<uint32_t>, SOUND_COUNT> triggers_{};

    // Audio thread only
    std::array<uint32_t, SOUND_COUNT> triggers_seen_{};
    std::array<size_t, SOUND_COUNT> positions_{};   // past the end when stopped
    float ufo_gain_ = 0.0f;     // fade in and out to avoid clicks
    float amp_gain_ = 0.0f;
};

#endif
