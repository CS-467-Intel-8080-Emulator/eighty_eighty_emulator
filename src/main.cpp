#include "src/frontend/frontend.h"
#include "src/platform/fake_invaders/machine.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
#include <iostream>

int main() {
    try {
        Machine machine;
        Frontend frontend(
            machine.title(),
            machine.display_width(),
            machine.display_height(),
            machine.display_rotation(),
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
