#include "src/frontend/frontend.hpp"
#include "platform/invaders/invaders_machine.hpp"
#include "platform/invaders/invaders_rom.hpp"
#include "rom/rom_reader.hpp"
#include "state/state.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>

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

        // TODO: Get state file from arg input or user input
        const std::string state_file = "states/invaders.autosave.state";

        // Load machine state from file if it exists; on failure, start fresh
        try {
            if (machine.load_state(StateFile::read(state_file)))
                SDL_Log("Loaded save state: %s", state_file.c_str());
            else
                SDL_Log("Failed to load save state (Wrong size): %s", state_file.c_str());
        } catch (const std::exception& e) {
            SDL_Log("Failed to load save state. %s", e.what());
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

        // Autosave machine state on exit
        try {
            StateFile::write(state_file, machine.save_state());
            SDL_Log("Created save state: %s", state_file.c_str());
        } catch (const std::exception& e) {
            SDL_Log("Failed to create save state. %s", e.what());
        }
    } catch (const std::exception& e) {
        SDL_Log("%s", e.what());
        return 1;
    }

    return 0;
}
