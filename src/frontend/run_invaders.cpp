#include "frontend/frontend.hpp"
#include "platform/invaders/invaders_machine.hpp"
#include "rom/rom_reader.hpp"
#include "run_invaders.hpp"

#include <SDL3/SDL.h>

#include <exception>
#include <string>
#include <vector>

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
