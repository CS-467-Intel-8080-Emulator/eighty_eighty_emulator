#include "src/frontend/audio.hpp"
#include "src/frontend/frontend.hpp"
#include "platform/invaders/invaders_machine.hpp"
#include "platform/invaders/invaders_rom.hpp"
#include "rom/rom_reader.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
#include <string>
#include <utility>
#include <vector>

int main(int argc, char* argv[]) {

    // Read folder name
    const char* folder = (argc >= 2) ? argv[1] : "assets/roms/invaders";

    try {
        InvadersMachine machine;

        // Read the Space Invaders ROM files listed in platform/invaders/invaders_rom.hpp
        // TODO: Get the list from the machine (e.g., machine.rom_files())
        std::vector<RomFile> rom_files = read_rom_files(folder, INVADERS_ROM_FILES);

        // Load each file's bytes into memory at its designated address
        for (const RomFile& rom_file : rom_files) {
            bool is_loaded = machine.load_rom(rom_file.bytes, rom_file.address);
            if (!is_loaded) {
                SDL_Log("ROM file doesn't fit in memory: %s", rom_file.name);
                return 1;
            }
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
