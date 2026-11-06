#include "sound.hpp"

#include <algorithm>
#include <utility>

namespace {
    // 10 ms fade
    constexpr float FADE_STEP = 1.0f / (0.010f * InvadersSound::SAMPLE_RATE);

    float fade_toward(float gain, float target) {
        return gain < target ? std::min(gain + FADE_STEP, target) : std::max(gain - FADE_STEP, target);
    }
}

const int8_t InvadersSound::PORT3_SOUNDS[8] = {
    -1, SHOT, PLAYER_DEATH, INVADER_DEATH, EXTRA_LIFE, -1, -1, -1,
};
const int8_t InvadersSound::PORT5_SOUNDS[8] = {
    FLEET_1, FLEET_2, FLEET_3, FLEET_4, UFO_HIT, -1, -1, -1,
};

void InvadersSound::set_sample(Sound sound, std::vector<float> samples) {
    samples_[sound] = std::move(samples);
    positions_[sound] = (sound == UFO) ? 0 : samples_[sound].size();
}

void InvadersSound::write_port3(uint8_t value) {
    count_rising_edges(port3_.load(std::memory_order_relaxed), value, PORT3_SOUNDS);
    port3_.store(value, std::memory_order_release);
}

void InvadersSound::write_port5(uint8_t value) {
    count_rising_edges(port5_.load(std::memory_order_relaxed), value, PORT5_SOUNDS);
    port5_.store(value, std::memory_order_release);
}

void InvadersSound::count_rising_edges(uint8_t old_value, uint8_t new_value, const int8_t (&sounds)[8]) {
    const uint8_t rose = new_value & ~old_value;
    for (int bit = 0; bit < 8; bit++) {
        if ((rose & (1 << bit)) && sounds[bit] >= 0)
            triggers_[sounds[bit]].fetch_add(1, std::memory_order_release);
    }
}

void InvadersSound::render(float* out, int count) {
    const uint8_t port3 = port3_.load(std::memory_order_acquire);
    const float amp_target = (port3 & BIT_AMP_ENABLE) ? 1.0f : 0.0f;
    const float ufo_target = (port3 & BIT_UFO) ? 1.0f : 0.0f;

    // Restart one-shots triggered since the last render
    for (int s = 0; s < SOUND_COUNT; s++) {
        const uint32_t triggers = triggers_[s].load(std::memory_order_acquire);
        if (triggers != triggers_seen_[s]) {
            triggers_seen_[s] = triggers;
            positions_[s] = 0;
        }
    }

    const std::vector<float>& ufo = samples_[UFO];
    for (int i = 0; i < count; i++) {
        float mix = 0.0f;
        for (int s = 0; s < SOUND_COUNT; s++) {
            if (s != UFO && positions_[s] < samples_[s].size())
                mix += samples_[s][positions_[s]++];
        }

        ufo_gain_ = fade_toward(ufo_gain_, ufo_target);
        if (ufo_gain_ == 0.0f) {
            positions_[UFO] = 0;
        } else if (!ufo.empty()) {
            mix += ufo_gain_ * ufo[positions_[UFO]];
            positions_[UFO] = (positions_[UFO] + 1) % ufo.size();
        }

        amp_gain_ = fade_toward(amp_gain_, amp_target);
        out[i] = std::clamp(0.5f * amp_gain_ * mix, -1.0f, 1.0f);
    }
}
