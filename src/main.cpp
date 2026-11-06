#include "src/frontend/audio.hpp"
#include "src/frontend/frontend.hpp"
#include "platform/invaders/invaders_machine.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        SDL_Log("Usage: %s ROM_FILE, ROM_FILE, ...", argv[0]);
        return 1;
    }

    try {
        InvadersMachine machine;

        // Load each ROM file right after the previous one
        // Borrowed file loading pattern from CP/M main
        uint16_t offset = 0;
        for (int i = 1; i < argc; i++) {
            std::ifstream file(argv[i], std::ios::binary);
            std::vector<uint8_t> rom((std::istreambuf_iterator<char>(file)), {});
            if (rom.empty() || !machine.load_rom(rom, offset)) {
                SDL_Log("Unable to load ROM %s", argv[i]);
                return 1;
            }
            offset += rom.size();
        }

        Frontend frontend(
            machine.title(),
            machine.display_width(),
            machine.display_height(),
            0.0, // TODO: Remove rotation; let video layer handle it
            2
        );

        // Missing sound files stay silent
        const std::string sound_dir = "assets/sounds/invaders/";
        int samples_loaded = 0;
        for (int i = 0; i < InvadersSound::SOUND_COUNT; i++) {
            const std::string path = sound_dir + std::to_string(i) + ".wav";
            std::vector<float> samples = Audio::load_wav(path.c_str(), machine.audio_sample_rate());
            samples_loaded += !samples.empty();
            machine.sound().set_sample(InvadersSound::Sound(i), std::move(samples));
        }
        SDL_Log("Loaded %d of %d sound samples from %s",
                samples_loaded, int(InvadersSound::SOUND_COUNT), sound_dir.c_str());

        // Must be destroyed before the frontend calls SDL_Quit
        Audio audio(machine.audio_sample_rate(), [&machine](float* out, int count) {
            machine.render_audio(out, count);
        });

        while (frontend.handle_events()) {
            machine.run_frame();
            frontend.present(machine.pixels());
        }
    } catch (const std::exception& e) {
        SDL_Log("%s", e.what());
        return 1;
    }

    return 0;
}
