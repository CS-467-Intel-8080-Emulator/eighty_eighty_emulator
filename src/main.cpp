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
            machine.get_title(),
            machine.get_display_width(),
            machine.get_display_height(),
            machine.get_display_rotation(),
            3
        );

        while (frontend.handle_events()) {
            machine.run_frame();
            frontend.present(machine.get_pixels());
        }
    } catch (const std::exception& e) {
        SDL_Log("%s", e.what());
        return 1;
    }

    return 0;
}
