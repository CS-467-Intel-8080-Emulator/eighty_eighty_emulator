#ifndef INVADERS_SOUND_FILES_HPP
#define INVADERS_SOUND_FILES_HPP

#include <vector>

#include "platform/sound_file.hpp"
#include "sound.hpp"

// Space Invaders sound files: one WAV per sound, read from assets/sounds/invaders.
// Players supply these files themselves. A missing file means that sound is silent.
inline const std::vector<SoundFile> INVADERS_SOUND_FILES = {
    {"0.wav", InvadersSound::UFO},
    {"1.wav", InvadersSound::SHOT},
    {"2.wav", InvadersSound::PLAYER_DEATH},
    {"3.wav", InvadersSound::INVADER_DEATH},
    {"4.wav", InvadersSound::FLEET_1},
    {"5.wav", InvadersSound::FLEET_2},
    {"6.wav", InvadersSound::FLEET_3},
    {"7.wav", InvadersSound::FLEET_4},
    {"8.wav", InvadersSound::UFO_HIT},
    {"9.wav", InvadersSound::EXTRA_LIFE},
};

#endif
