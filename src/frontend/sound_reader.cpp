// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). This document summarizes the GenAI usage that led to my code:
// https://drive.google.com/file/d/1SDeaM_7hkEwhuc5EPaecAapZURuSGOfO/view?usp=sharing

#include "sound_reader.hpp"

#include "audio.hpp"

#include <SDL3/SDL.h>

std::vector<SoundFile> read_sound_files(const std::string& folder, std::vector<SoundFile> files, int sample_rate) {
    if (files.empty())
        return files;

    int loaded = 0;
    for (SoundFile& file : files) {
        const std::string path = folder + "/" + file.name;
        file.samples = Audio::load_wav(path.c_str(), sample_rate);
        if (!file.samples.empty())
            loaded++;
    }

    SDL_Log("Loaded %d of %d sound files from %s", loaded, int(files.size()), folder.c_str());
    return files;
}
