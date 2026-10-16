#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>

#include "src/platform/fake_invaders/machine.h"

constexpr int DISPLAY_SCALE = 3;

bool init_sdl(const char* title, int width, int height);
void quit_sdl();
bool poll_input();
double get_time_in_ms();

SDL_Window* g_window{ nullptr };
SDL_Renderer* g_renderer{ nullptr };
SDL_Texture* g_texture{ nullptr };

// int main(int argc, char* argv[]) {
    // (void)argc; (void)argv;
int main() {
    Machine machine;

    const int width = machine.get_display_width();
    const int height = machine.get_display_height();
    const double frame_duration_ms = 1000.0 / machine.get_framerate();

    if (!init_sdl(machine.get_title(), width, height)) {
        return 1;
    }

    double prev_frame_time = get_time_in_ms();
    bool running = true;

    while (running) {
        if (!poll_input()) {
            running = false;
            continue;
        }

        const double now = get_time_in_ms();

        if (now - prev_frame_time >= frame_duration_ms) {
            machine.run_frame();

            const uint32_t* pixels = machine.get_display_buffer();

            SDL_RenderClear(g_renderer);
            SDL_UpdateTexture(g_texture, NULL, pixels, width * sizeof(uint32_t));
            SDL_RenderTexture(g_renderer, g_texture, NULL, NULL);
            SDL_RenderPresent(g_renderer);

            prev_frame_time += frame_duration_ms;
        }
    }

    quit_sdl();
    return 0;
}

double get_time_in_ms() {
    return static_cast<double>(SDL_GetTicksNS()) / 1'000'000.0;
}

bool poll_input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
            return false;
        }
    }

    return true;
}

bool init_sdl(const char* title, int width, int height) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL was not initialized. SDL error: %s\n", SDL_GetError());
        return false;
    }

    g_window = SDL_CreateWindow(title, width * DISPLAY_SCALE, height * DISPLAY_SCALE, 0);
    g_renderer = g_window ? SDL_CreateRenderer(g_window, NULL) : nullptr;
    g_texture = g_renderer ? SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, width, height)
        : nullptr;

    if (!g_window || !g_renderer || !g_texture) {
        SDL_Log("Window/renderer/texture not initialized. SDL error: %s\n", SDL_GetError());
        quit_sdl();
        return false;
    }

    SDL_SetTextureScaleMode(g_texture, SDL_SCALEMODE_NEAREST);
    return true;
}

void quit_sdl() {
    if (g_texture) { SDL_DestroyTexture(g_texture);   g_texture = nullptr; }
    if (g_renderer) { SDL_DestroyRenderer(g_renderer); g_renderer = nullptr; }
    if (g_window) { SDL_DestroyWindow(g_window);     g_window = nullptr; }

    SDL_Quit();
}
