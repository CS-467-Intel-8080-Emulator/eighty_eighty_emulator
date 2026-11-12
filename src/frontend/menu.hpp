#ifndef MENU_HPP
#define MENU_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <vector>

// ======================================================
// ROM Selection Menu
// ======================================================
//
// Discovers available ROM sets, renders the front-end
// menu, and tracks the user's current selection.
//
class Menu
{
public:
    Menu();
    ~Menu();

    // --------------------------------------------------
    // Input and Rendering
    // --------------------------------------------------

    void handle_input(const SDL_Event& event);
    void render(SDL_Renderer* renderer);

    // Called after SDL_ttf initialization to load
    // font resources used by the menu.
    void initialize();

    // --------------------------------------------------
    // Menu State
    // --------------------------------------------------

    const std::vector<std::string>& games() const;

    std::string selected_game() const;
    bool game_selected() const;

private:

    // --------------------------------------------------
    // ROM Discovery
    // --------------------------------------------------

    void discover_games();

    // Names of ROM directories found in assets/roms.
    std::vector<std::string> games_;

    // Currently highlighted ROM index.
    int selected_;

    // Set when the user confirms a selection.
    bool game_selected_;

    // Font used to render menu text.
    TTF_Font* font_;
};

#endif
