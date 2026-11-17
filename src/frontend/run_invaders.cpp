#include "frontend/frontend.hpp"
#include "platform/invaders/invaders_machine.hpp"
#include "rom/rom_reader.hpp"
#include "run_invaders.hpp"
#include "src/frontend/audio.hpp"
#include "src/frontend/sound_reader.hpp"

#include <SDL3/SDL.h>

#include <exception>
#include <string>
#include <vector>
#include <utility>

// Run Space Invaders by loading the ROMS in `folder`
int run_invaders(const std::string& folder) {

    try {
        InvadersMachine machine;

        // Read the required ROM files from `folder`
        std::vector<RomFile> rom_files = read_rom_files(folder, machine.rom_files());

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

        frontend.set_input_handler([&machine](SDL_Scancode key, bool pressed) {
            for (const KeyBinding& binding : KEY_BINDINGS) {
                if (SDL_GetScancodeFromName(binding.key) == key) {
                    machine.set_input(binding.input, pressed);
                    break;
                }
            }
        });

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
