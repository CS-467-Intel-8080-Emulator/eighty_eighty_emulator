#include "src/frontend/audio.hpp"
#include "src/frontend/frontend.hpp"
#include "src/frontend/sound_reader.hpp"
#include "src/frontend/asset_paths.hpp"

#include "platform/invaders/invaders_machine.hpp"
#include "platform/invaders/invaders_rom.hpp"
#include "rom/rom_reader.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <exception>
#include <string>
#include <utility>
#include <vector>

// AI was used to debug the game selection function and clean
// load/run ROMs function.

int main(int argc, char* argv[])
{
    int result = 0;

    try
    {
        // -----------------------------------------------------------------
        // Show ROM selection menu
        // -----------------------------------------------------------------

        // TODO: Menu currently uses Space Invaders display
        // dimensions. Generalize this when supporting multiple machine types.
        Frontend frontend(
            "8080 Emulator",
            224,
            256,
            0.0,
            2);

        std::string selected_game;

        while (frontend.handle_events())
        {
            frontend.render_menu();

            if (frontend.game_selected())
            {
                selected_game = frontend.selected_game();
                break;
            }
        }

        // User closed the window without selecting a ROM.
        if (!selected_game.empty())
        {
            // -------------------------------------------------------------
            // Load selected ROM
            // -------------------------------------------------------------

            const auto rom_path =
                get_rom_root() /
                selected_game;

            InvadersMachine machine;

            std::vector<RomFile> rom_files =
                read_rom_files(
                    rom_path.string().c_str(),
                    INVADERS_ROM_FILES);

            for (const RomFile& rom_file : rom_files)
            {
                bool is_loaded =
                    machine.load_rom(
                        rom_file.bytes,
                        rom_file.address);

                if (!is_loaded)
                {
                    SDL_Log(
                        "ROM file doesn't fit in memory: %s",
                        rom_file.name);

                    result = 1;
                    break;
                }
            }

            if (result == 0)
            {
                // ---------------------------------------------------------
                // Input bindings
                // ---------------------------------------------------------

                frontend.set_input_handler(
                    [&machine](SDL_Scancode key, bool pressed)
                    {

                        for (const KeyBinding& binding : KEY_BINDINGS)
                        {
                            if (SDL_GetScancodeFromName(binding.key) == key)
                            {
                                machine.set_input(
                                    binding.input,
                                    pressed);

                                break;
                            }
                        }
                    });

                // ---------------------------------------------------------
                // Audio
                // ---------------------------------------------------------

                const auto sound_path =
                    asset_root() /
                    "sounds" /
                    "invaders";

                std::vector<SoundFile> sound_files =
                    read_sound_files(
                        sound_path.string().c_str(),
                        machine.sound_files(),
                        machine.audio_sample_rate());

                for (SoundFile& sound_file : sound_files)
                {
                    machine.load_sound(
                        sound_file.id,
                        std::move(sound_file.samples));
                }

                // Must be destroyed before SDL subsystems shut down.
                Audio audio(
                    machine.audio_sample_rate(),
                    [&machine](float* out, int count)
                    {
                        machine.render_audio(out, count);
                    });

                // ---------------------------------------------------------
                // Run emulator
                // ---------------------------------------------------------

                while (frontend.handle_events())
                {
                    machine.run_frame();
                    frontend.present(machine.pixels());
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        SDL_Log("%s", e.what());
        result = 1;
    }

    TTF_Quit();
    SDL_Quit();

    return result;
}