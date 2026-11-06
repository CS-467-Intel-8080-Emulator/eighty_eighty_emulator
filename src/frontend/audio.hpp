#pragma once

#include <SDL3/SDL.h>

#include <functional>
#include <vector>

// Mono audio output. SDL calls `render` from its audio thread.
// If no device opens, the program runs without sound.
class Audio {
public:
    using RenderFn = std::function<void(float* out, int count)>;

    Audio(int sample_rate, RenderFn render);
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    // Returns an empty vector if the file is missing or unreadable
    static std::vector<float> load_wav(const char* path, int sample_rate);

private:
    static void SDLCALL on_audio_needed(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);

    RenderFn render_;
    SDL_AudioStream* stream_{ nullptr };
};
