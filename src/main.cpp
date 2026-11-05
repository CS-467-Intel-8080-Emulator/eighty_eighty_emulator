#include "src/frontend/frontend.hpp"
#include "platform/invaders/invaders_machine.hpp"
#include "platform/invaders/invaders_rom.hpp"
#include "rom/rom_loader.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>

int main(int argc, char* argv[]) {
    const char* folder = (argc >= 2) ? argv[1] : "assets/roms/invaders";

    try {
        InvadersMachine machine;

        // Load each ROM file at its address from INVADERS_ROM_FILES
        RomLoadResult rom = load_rom_files(folder, INVADERS_ROM_FILES);
        if (!rom.ok()) {
            SDL_Log("%s", rom.error.c_str());
            return 1;
        }
        for (size_t i = 0; i < rom.data.size(); i++)
            machine.load_rom(rom.data[i], INVADERS_ROM_FILES[i].address);

        Frontend frontend(
            machine.title(),
            machine.display_width(),
            machine.display_height(),
            0.0, // TODO: Remove rotation; let video layer handle it
            2
        );

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
