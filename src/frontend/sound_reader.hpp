#ifndef SOUND_READER_HPP
#define SOUND_READER_HPP

#include <string>
#include <vector>

#include "platform/sound_file.hpp"

// Reads each sound file in folder, converted to mono at sample_rate.
// A missing or unreadable file leaves its samples empty.
std::vector<SoundFile> read_sound_files(const std::string& folder, std::vector<SoundFile> files, int sample_rate);

#endif
