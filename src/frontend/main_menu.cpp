#include "main_menu.hpp"
#include "run_invaders.hpp"

int run_main_menu() {
    // TODO: Add a main menu UI for choosing games and setting options
    // With no UI and only one game right now, load Space Invaders directly instead
    int result = run_invaders("assets/roms/invaders");
    return result;
}
