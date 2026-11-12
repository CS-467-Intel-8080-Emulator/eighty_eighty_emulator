#include "src/frontend/frontend.hpp"
#include "platform/invaders/invaders_machine.hpp"
#include "platform/invaders/invaders_rom.hpp"
#include "rom/rom_reader.hpp"
#include "src/frontend/asset_paths.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
#include <string>

// AI was used to debug the game selection function.

int main(int argc, char* argv[])
{
    int result = 0;

    try
    {
        // -----------------------------------------------------------------
        // Show ROM selection menu
        // -----------------------------------------------------------------

        Frontend frontend(
            "8080 Emulator",
            224,
            256,
            0.0,
            2
            // TODO: Menu currently uses Space Invaders display
            // dimensions. Generalize this when supporting multiple machine types.
        );

        // Directory name of the selected ROM set (e.g. "invaders").
        std::string selected_game;

        while (frontend.handle_events())
        {
            frontend.render_menu();

            if (frontend.game_selected())
            {
                selected_game =
                    frontend.selected_game();

                break;
            }
        }

        // User closed the window without selecting a ROM.
        if (selected_game.empty())
        {
            result = 0;
        }
        else
        {
            // Build the full path to the selected ROM set.
            const auto rom_path =
                asset_root() /
                "roms" /
                selected_game;

            // -----------------------------------------------------------------
            // Load selected ROM
            // -----------------------------------------------------------------

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

            // -----------------------------------------------------------------
            // Run emulator
            // -----------------------------------------------------------------

            if (result == 0)
            {
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
