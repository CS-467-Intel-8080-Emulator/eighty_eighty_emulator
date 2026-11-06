// Unit tests for the Space Invaders sound board, using test samples in place of recorded ones.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/platform/invaders/sound.hpp"

using Catch::Matchers::WithinAbs;

namespace {
    constexpr uint8_t AMP = 0x20;           // port 3 bit 5
    constexpr uint8_t UFO = 0x01;           // port 3 bit 0
    constexpr uint8_t SHOT = 0x02;          // port 3 bit 1

    constexpr int SHORT = InvadersSound::SAMPLE_RATE / 20;             // 50 ms
    constexpr int SAMPLE_LENGTH = InvadersSound::SAMPLE_RATE / 10;     // 100 ms
    constexpr int LONG = InvadersSound::SAMPLE_RATE;                   // 1 s
    static_assert(SHORT < SAMPLE_LENGTH && SAMPLE_LENGTH < LONG);

    std::vector<float> flat(int length = SAMPLE_LENGTH) { return std::vector<float>(length, 0.5f); }

    // Rising values, so the output shows how far into the sample playback is
    float ramp_value(int i) { return 0.1f + i * 0.00001f; }
    std::vector<float> ramp(int length) {
        std::vector<float> samples(length);
        for (int i = 0; i < length; i++)
            samples[i] = ramp_value(i);
        return samples;
    }

    void load_test_samples(InvadersSound& sound) {
        for (int s = 0; s < InvadersSound::SOUND_COUNT; s++)
            sound.set_sample(InvadersSound::Sound(s), flat());
    }

    std::vector<float> render(InvadersSound& sound, int count) {
        std::vector<float> out(count);
        sound.render(out.data(), count);
        return out;
    }

    float peak(InvadersSound& sound, int count) {
        float max = 0.0f;
        for (float s : render(sound, count))
            max = std::max(max, std::abs(s));
        return max;
    }

    bool sounds(InvadersSound& sound, int count = SHORT) { return peak(sound, count) > 0.01f; }
}


// --- Amplifier ---

TEST_CASE("Sound board is silent at power on", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    CHECK_FALSE(sounds(sound, LONG));
}

TEST_CASE("Amplifier enable alone makes no sound", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(AMP);
    CHECK_FALSE(sounds(sound, LONG));
}

TEST_CASE("Sounds stay silent while the amplifier is off", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(SHOT | UFO);
    sound.write_port5(0x1F);
    CHECK_FALSE(sounds(sound));
}

TEST_CASE("Amplifier fades in", "[invaders][sound]") {
    InvadersSound sound;
    sound.set_sample(InvadersSound::SHOT, flat());
    sound.write_port3(AMP | SHOT);

    std::vector<float> out = render(sound, 1000);
    CHECK(out[0] > 0.0f);
    CHECK(out[0] < 0.01f);
    CHECK(out[999] == 0.25f);               // 0.5 sample at 0.5 mix level
}

TEST_CASE("Turning the amplifier off silences a playing sound", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(AMP | UFO);
    REQUIRE(sounds(sound));

    sound.write_port3(UFO);
    peak(sound, SHORT);                     // fade out
    CHECK_FALSE(sounds(sound));
}


// --- One-shot sounds ---

TEST_CASE("Each one-shot bit plays its own sound once", "[invaders][sound]") {
    auto [port, bit, expected] = GENERATE(table<int, uint8_t, InvadersSound::Sound>({
        { 3, 0x02, InvadersSound::SHOT },
        { 3, 0x04, InvadersSound::PLAYER_DEATH },
        { 3, 0x08, InvadersSound::INVADER_DEATH },
        { 3, 0x10, InvadersSound::EXTRA_LIFE },
        { 5, 0x01, InvadersSound::FLEET_1 },
        { 5, 0x02, InvadersSound::FLEET_2 },
        { 5, 0x04, InvadersSound::FLEET_3 },
        { 5, 0x08, InvadersSound::FLEET_4 },
        { 5, 0x10, InvadersSound::UFO_HIT },
    }));
    CAPTURE(port, (int)bit, (int)expected);

    // Only the expected sound has a sample, so the wrong one would be silent
    InvadersSound sound;
    sound.set_sample(expected, flat());
    sound.write_port3(AMP);
    if (port == 3)
        sound.write_port3(AMP | bit);
    else
        sound.write_port5(bit);

    CHECK(sounds(sound));
    peak(sound, LONG);                      // bit still set
    CHECK_FALSE(sounds(sound));
}

TEST_CASE("A one-shot plays its sample to the end", "[invaders][sound]") {
    InvadersSound sound;
    sound.set_sample(InvadersSound::SHOT, flat(100));
    sound.write_port3(AMP);
    peak(sound, SHORT);

    sound.write_port3(AMP | SHOT);
    std::vector<float> out = render(sound, 200);
    CHECK(out[0] == 0.25f);
    CHECK(out[99] == 0.25f);
    CHECK(out[100] == 0.0f);
}

TEST_CASE("Holding a one-shot bit set does not replay it", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(AMP | SHOT);
    peak(sound, LONG);

    sound.write_port3(AMP | SHOT);
    CHECK_FALSE(sounds(sound));
}

TEST_CASE("Clearing and setting a one-shot bit replays it", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(AMP | SHOT);
    peak(sound, LONG);

    sound.write_port3(AMP);
    sound.write_port3(AMP | SHOT);
    CHECK(sounds(sound));
}

TEST_CASE("Retriggering a one-shot restarts it from the beginning", "[invaders][sound]") {
    InvadersSound sound;
    sound.set_sample(InvadersSound::SHOT, ramp(SAMPLE_LENGTH));
    sound.write_port3(AMP);
    peak(sound, SHORT);

    sound.write_port3(AMP | SHOT);
    render(sound, 1000);
    sound.write_port3(AMP);
    sound.write_port3(AMP | SHOT);
    CHECK_THAT(render(sound, 1)[0], WithinAbs(0.5f * ramp_value(0), 1e-6));
}

TEST_CASE("A bit set and cleared between renders still plays", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(AMP);
    peak(sound, SHORT);

    sound.write_port3(AMP | SHOT);
    sound.write_port3(AMP);
    CHECK(sounds(sound));
}

TEST_CASE("Screen flip bit makes no sound", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(AMP);
    sound.write_port5(0x20);                // port 5 bit 5
    CHECK_FALSE(sounds(sound, LONG));
}

TEST_CASE("A sound without a sample is silent", "[invaders][sound]") {
    InvadersSound sound;
    sound.write_port3(AMP | UFO | SHOT);
    sound.write_port5(0x1F);
    CHECK_FALSE(sounds(sound, LONG));
}


// --- UFO loop ---

TEST_CASE("UFO loops while its bit is set and stops when cleared", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(AMP | UFO);
    peak(sound, LONG);
    CHECK(sounds(sound));

    sound.write_port3(AMP);
    peak(sound, SHORT);
    CHECK_FALSE(sounds(sound));
}

TEST_CASE("UFO restarts its sample each time it turns on", "[invaders][sound]") {
    InvadersSound sound;
    sound.set_sample(InvadersSound::UFO, ramp(LONG));
    sound.write_port3(AMP);
    peak(sound, SHORT);

    sound.write_port3(AMP | UFO);
    peak(sound, SHORT);
    sound.write_port3(AMP);
    peak(sound, SHORT);

    sound.write_port3(AMP | UFO);
    std::vector<float> out = render(sound, 1000);
    CHECK_THAT(out[999], WithinAbs(0.5f * ramp_value(999), 1e-6));
}


// --- Output range ---

TEST_CASE("Output is clamped to [-1, 1]", "[invaders][sound]") {
    InvadersSound sound;
    load_test_samples(sound);
    sound.write_port3(0x3F);
    sound.write_port5(0x1F);
    CHECK(peak(sound, LONG) <= 1.0f);
}
