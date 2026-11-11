#pragma once

#include "menu.hpp"

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

// ======================================================
// Frontend
// ======================================================
// Manages SDL initialization, rendering, user input, and the ROM selection menu.
class Frontend {
public:

    // --------------------------------------------------
    // Construction and Cleanup
    // --------------------------------------------------

    Frontend(const char* title,
             int w_native,
             int h_native,
             double rotation,
             int scale);

    ~Frontend();

    Frontend(const Frontend&) = delete;
    Frontend& operator=(const Frontend&) = delete;

    // --------------------------------------------------
    // Input and Rendering
    // --------------------------------------------------

    bool handle_events();

    void present(const uint32_t* pixels);
    void render_menu();

    // --------------------------------------------------
    // Menu State
    // --------------------------------------------------

    bool game_selected() const;
    std::string selected_game() const;

private:

    // --------------------------------------------------
    // Internal Cleanup
    // --------------------------------------------------

    void cleanup();

    // SDL rendering resources.
    SDL_Window* window_{ nullptr };
    SDL_Renderer* renderer_{ nullptr };
    SDL_Texture* texture_{ nullptr };

    // Destination rectangle used when rendering
    // the emulated display.
    SDL_FRect dst_{};

    // Display configuration.
    double rotation_{ 0.0 };
    int w_native_{ 0 };

    // ROM selection menu.
    Menu menu_;
};
