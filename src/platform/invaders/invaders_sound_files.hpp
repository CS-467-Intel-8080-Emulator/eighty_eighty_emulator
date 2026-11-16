// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). This document summarizes the GenAI usage that led to my code:
// https://drive.google.com/file/d/1SDeaM_7hkEwhuc5EPaecAapZURuSGOfO/view?usp=sharing

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
