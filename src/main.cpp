#include "src/frontend/frontend.hpp"
#include "platform/invaders/invaders_machine.hpp"
#include "platform/invaders/invaders_rom.hpp"
#include "rom/rom_reader.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
#include <string>

// AI was used to debug the game selection function.

int main(int argc, char* argv[])
{
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
        );

        std::string selected_folder;

        while (frontend.handle_events())
        {
            frontend.render_menu();

            if (frontend.game_selected())
            {
                selected_folder =
                    "../assets/roms/" +
                    frontend.selected_game();

                break;
            }
        }

        // User closed window without selecting anything
        if (selected_folder.empty())
        {
            return 0;
        }

        // -----------------------------------------------------------------
        // Load selected ROM
        // -----------------------------------------------------------------

        InvadersMachine machine;

        std::vector<RomFile> rom_files =
            read_rom_files(
                selected_folder.c_str(),
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

                return 1;
            }
        }

        // -----------------------------------------------------------------
        // Run emulator
        // -----------------------------------------------------------------

        while (frontend.handle_events())
        {
            machine.run_frame();
            frontend.present(machine.pixels());
        }
    }
    catch (const std::exception& e)
    {
        SDL_Log("%s", e.what());
        return 1;
    }

    return 0;
}
