#include "frontend.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdexcept>
#include <string>

// AI used to help refactor for new menu interface and rendering with SDL3.

// ======================================================
// Frontend Construction
// ======================================================
Frontend::Frontend(const char* title,
                   int w_native,
                   int h_native,
                   double rotation,
                   int scale)
    : w_native_{ w_native },
      rotation_(rotation)
{

    // Adjust the logical display dimensions when rendering a rotated game screen.
    const bool sideways =
        (rotation == 90.0 || rotation == 270.0);

    const int w_logical =
        sideways ? h_native : w_native;

    const int h_logical =
        sideways ? w_native : h_native;

    // Calculate the destination rectangle used to center the emulated display.
    dst_.w = static_cast<float>(w_native);
    dst_.h = static_cast<float>(h_native);

    dst_.x =
        (w_logical - dst_.w) / 2.0f;

    dst_.y =
        (h_logical - dst_.h) / 2.0f;
    
        // Initialize SDL video support.
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error(
            std::string(
                "SDL was not initialized. SDL error: ")
            + SDL_GetError());
    }
    
    // Initialize SDL_ttf before loading menu fonts.
    if (!TTF_Init())
    {
        SDL_Quit();

        throw std::runtime_error(
            std::string(
                "SDL_ttf was not initialized. SDL error: ")
            + SDL_GetError());
    }

    // Load menu resources now that SDL_ttf is ready.
    menu_.initialize();

    // Create the SDL window, renderer, and texture used for emulator output.
    window_ =
        SDL_CreateWindow(
            title,
            w_logical * scale,
            h_logical * scale,
            SDL_WINDOW_RESIZABLE);

    renderer_ =
        window_
            ? SDL_CreateRenderer(window_, nullptr)
            : nullptr;

    texture_ =
        renderer_
            ? SDL_CreateTexture(
                  renderer_,
                  SDL_PIXELFORMAT_ARGB8888,
                  SDL_TEXTUREACCESS_STREAMING,
                  w_native,
                  h_native)
            : nullptr;

    // Abort startup if any rendering resource could not be created.
    if (!window_ || !renderer_ || !texture_)
    {
        std::string msg =
            std::string(
                "Window/renderer/texture not initialized. SDL error: ")
            + SDL_GetError();

        cleanup();

        throw std::runtime_error(msg);
    }

    // Configure renderer behavior and preserve the native arcade aspect ratio.
    SDL_SetWindowMinimumSize(
        window_,
        w_logical,
        h_logical);

    SDL_SetTextureScaleMode(
        texture_,
        SDL_SCALEMODE_NEAREST);

    SDL_SetRenderVSync(
        renderer_,
        1);

    SDL_SetRenderLogicalPresentation(
        renderer_,
        w_logical,
        h_logical,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

// ======================================================
// Cleanup
// ======================================================
Frontend::~Frontend()
{
    cleanup();
}

// ======================================================
// Input Handling
// ======================================================
bool Frontend::handle_events()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        // Close the application when the window is closed by the user.
        if (event.type == SDL_EVENT_QUIT)
        {
            return false;
        }

        // Escape exits the emulator.
        if (event.type == SDL_EVENT_KEY_DOWN &&
            event.key.scancode == SDL_SCANCODE_ESCAPE)
        {
            return false;
        }

        menu_.handle_input(event);
    }

    return true;
}

// ======================================================
// Menu Rendering
// ======================================================
void Frontend::render_menu()
{
    // Clear the screen and draw the ROM selection menu.
    SDL_SetRenderDrawColor(
        renderer_,
        0,
        0,
        0,
        255);

    SDL_RenderClear(renderer_);

    menu_.render(renderer_);

    SDL_RenderPresent(renderer_);
}

// ======================================================
// Emulator Rendering
// ======================================================
void Frontend::present(
    const uint32_t* pixels)
{
    // Upload the latest frame produced by the emulator.
    SDL_UpdateTexture(
        texture_,
        nullptr,
        pixels,
        w_native_ * sizeof(uint32_t));

    SDL_RenderClear(renderer_);

    SDL_RenderTextureRotated(
        renderer_,
        texture_,
        nullptr,
        &dst_,
        rotation_,
        nullptr,
        SDL_FLIP_NONE);

    SDL_RenderPresent(renderer_);
}


// ======================================================
// Menu State
// ======================================================
bool Frontend::game_selected() const
{
    return menu_.game_selected();
}

std::string Frontend::selected_game() const
{
    return menu_.selected_game();
}

// ======================================================
// Internal Cleanup
// ======================================================
void Frontend::cleanup()
{
    if (texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }

    if (renderer_)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    // Shut down SDL subsystems.
    TTF_Quit();
    SDL_Quit();
}
