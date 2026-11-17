#include "src/frontend/audio.hpp"
#include "src/frontend/frontend.hpp"
#include "src/frontend/sound_reader.hpp"
#include "platform/invaders/invaders_machine.hpp"
#include "platform/invaders/invaders_rom.hpp"
#include "rom/rom_reader.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
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

        // Read the sound files the machine lists. Missing files stay silent.
        const char* sound_folder = "assets/sounds/invaders";
        std::vector<SoundFile> sound_files =
            read_sound_files(sound_folder, machine.sound_files(), machine.audio_sample_rate());
        for (SoundFile& sound_file : sound_files)
            machine.load_sound(sound_file.id, std::move(sound_file.samples));

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
