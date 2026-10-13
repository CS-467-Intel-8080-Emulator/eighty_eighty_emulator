#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    const int v = SDL_GetVersion();
    std::cout << "Hello from SDL "
        << SDL_VERSIONNUM_MAJOR(v) << '.'
        << SDL_VERSIONNUM_MINOR(v) << '.'
        << SDL_VERSIONNUM_MICRO(v) << '\n';

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("Hello, World!", 640, 480, 0, &window, &renderer)) {
        std::cerr << "Window/renderer creation failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    // The debug font is only 8x8 pixels, so scale everything up 4x
    SDL_SetRenderScale(renderer, 4.0f, 4.0f);

    const char* text = "Hello, world!";
    // Coordinates are in scaled units: window is 640x480 / 4 = 160x120
    const float textW = 13 * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;  // 13 characters
    const float textH = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    const float x = (160.0f - textW) / 2.0f;
    const float y = (120.0f - textH) / 2.0f;

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

        // Black background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Green text
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDebugText(renderer, x, y, text);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}