#include "audio.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

Audio::Audio(int sample_rate, RenderFn render)
    : render_{ std::move(render) } {

    if (sample_rate <= 0)
        return;

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("Audio was not initialized; continuing without sound. SDL error: %s", SDL_GetError());
        return;
    }

    const SDL_AudioSpec spec{ SDL_AUDIO_F32, 1, sample_rate };
    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, on_audio_needed, this);
    if (!stream_) {
        SDL_Log("No audio device opened; continuing without sound. SDL error: %s", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }

    if (!SDL_ResumeAudioStreamDevice(stream_))
        SDL_Log("Unable to start audio. SDL error: %s", SDL_GetError());
}

Audio::~Audio() {
    if (stream_) {
        SDL_DestroyAudioStream(stream_);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
}

std::vector<float> Audio::load_wav(const char* path, int sample_rate) {
    if (!SDL_GetPathInfo(path, nullptr))
        return {};

    SDL_AudioSpec src_spec;
    Uint8* src = nullptr;
    Uint32 src_len = 0;
    if (!SDL_LoadWAV(path, &src_spec, &src, &src_len)) {
        SDL_Log("Unable to load sound %s. SDL error: %s", path, SDL_GetError());
        return {};
    }

    const SDL_AudioSpec dst_spec{ SDL_AUDIO_F32, 1, sample_rate };
    Uint8* dst = nullptr;
    int dst_len = 0;
    const bool converted = SDL_ConvertAudioSamples(&src_spec, src, int(src_len), &dst_spec, &dst, &dst_len);
    SDL_free(src);
    if (!converted) {
        SDL_Log("Unable to convert sound %s. SDL error: %s", path, SDL_GetError());
        return {};
    }

    const float* first = reinterpret_cast<const float*>(dst);
    std::vector<float> samples(first, first + dst_len / sizeof(float));
    SDL_free(dst);
    return samples;
}

void SDLCALL Audio::on_audio_needed(void* userdata, SDL_AudioStream* stream, int additional_amount, int /*total_amount*/) {
    auto* self = static_cast<Audio*>(userdata);
    float buffer[512];

    // additional_amount is in bytes
    int remaining = additional_amount / int(sizeof(float));
    while (remaining > 0) {
        const int count = std::min(remaining, int(std::size(buffer)));
        self->render_(buffer, count);
        SDL_PutAudioStreamData(stream, buffer, count * int(sizeof(float)));
        remaining -= count;
    }
}
