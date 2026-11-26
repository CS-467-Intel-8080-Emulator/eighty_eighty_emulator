#include "run_menu.hpp"

#include "frontend/asset_paths.hpp"
#include "frontend/frontend.hpp"
#include "frontend/run_invaders.hpp"

#include <SDL3/SDL.h>

#include <exception>
#include <filesystem>
#include <string>

// AI was used to debug the game selection function and clean
// load/run ROMs function.

// Show the ROM selection menu and return the chosen game's folder name.
// Returns an empty string if the user closes the window without choosing.
// The menu window closes when this returns, before the game opens its own.
static std::string choose_game()
{
    // TODO: Menu currently uses Space Invaders display
    // dimensions. Generalize this when supporting multiple machine types.
    Frontend frontend(
        "8080 Emulator",
        224,
        256,
        0.0,
        2);

    while (frontend.handle_events())
    {
        frontend.render_menu();

        bool is_selected = frontend.game_selected();
        if (is_selected)
        {
            std::string selected_game = frontend.selected_game();
            return selected_game;
        }
    }

    return "";
}

int run_menu() {
    int result = 0;

    try
    {
        std::string selected_game = choose_game();

        // Empty if the user closed the window without selecting a ROM.
        bool user_chose_game = !selected_game.empty();
        if (user_chose_game)
        {
            std::filesystem::path rom_folder = get_rom_root() / selected_game;
            result = run_invaders(rom_folder.string());
        }
    }
    catch (const std::exception& e)
    {
        SDL_Log("%s", e.what());
        result = 1;
    }

    return result;
}
