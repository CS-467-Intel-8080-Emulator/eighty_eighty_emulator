#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>

#include "src/platform/fake_invaders/machine.h"

constexpr int SCALE = 3;

bool init_sdl(const char* title, int w_native, int h_native, int w_out, int h_out);
void quit_sdl();
bool handle_events();

SDL_Window* window{ nullptr };
SDL_Renderer* renderer{ nullptr };
SDL_Texture* texture{ nullptr };

int main() {
    Machine machine;

    const int w_native = machine.get_display_width();
    const int h_native = machine.get_display_height();

    const double rotation = machine.get_display_rotation();
    const bool sideways = (rotation == 90.0 || rotation == 270.0);

    const int w_logical = (sideways ? h_native : w_native);
    const int h_logical = (sideways ? w_native : h_native);

    SDL_FRect dst;
    dst.w = static_cast<float>(w_native);
    dst.h = static_cast<float>(h_native);
    dst.x = (w_logical - dst.w) / 2.0f;
    dst.y = (h_logical - dst.h) / 2.0f;

    if (!init_sdl(machine.get_title(), w_native, h_native, w_logical, h_logical))
        return 1;

    bool running = true;

    while (running) {
        running = handle_events();

        machine.run_frame();
        const uint32_t* pixels = machine.get_pixels();

        SDL_UpdateTexture(texture, nullptr, pixels, w_native * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderTextureRotated(renderer, texture, nullptr, &dst, rotation, nullptr, SDL_FLIP_NONE);
        SDL_RenderPresent(renderer);
    }

    quit_sdl();
    return 0;
}

bool handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            return false;

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE)
            return false;
    }

    return true;
}

bool init_sdl(const char* title, int w_native, int h_native, int w_logical, int h_logical) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL was not initialized. SDL error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(title, w_logical, h_logical, SDL_WINDOW_RESIZABLE);
    renderer = window ? SDL_CreateRenderer(window, nullptr) : nullptr;
    texture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, w_native, h_native)
        : nullptr;

    if (!window || !renderer || !texture) {
        SDL_Log("Window/renderer/texture not initialized. SDL error: %s\n", SDL_GetError());
        quit_sdl();
        return false;
    }

    SDL_SetWindowMinimumSize(window, w_logical, h_logical);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderVSync(renderer, 1);  // Makes it so SDL_RenderPresent blocks
    SDL_SetRenderLogicalPresentation(renderer, w_logical, h_logical, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return true;
}

void quit_sdl() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}
