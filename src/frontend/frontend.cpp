#include "frontend.hpp"

#include <SDL3/SDL.h>

#include <stdexcept>
#include <string>

// keyboard key to arcade control mapping
namespace {
    struct KeyBinding {SDL_Scancode key; Input input;};
    const KeyBinding KEY_BINDINGS[] = {
        { SDL_SCANCODE_C,     Input::Coin },
        { SDL_SCANCODE_1,     Input::P1Start },
        { SDL_SCANCODE_2,     Input::P2Start },
        { SDL_SCANCODE_LEFT,  Input::P1Left },
        { SDL_SCANCODE_RIGHT, Input::P1Right },
        { SDL_SCANCODE_SPACE, Input::P1Fire },
        { SDL_SCANCODE_A,     Input::P2Left },
        { SDL_SCANCODE_D,     Input::P2Right },
        { SDL_SCANCODE_W,     Input::P2Fire },
        { SDL_SCANCODE_T,     Input::Tilt },
    };
}

Frontend::Frontend(const char* title, int w_native, int h_native, double rotation, int scale)
    : w_native_{ w_native }, rotation_(rotation) {

    const bool sideways = (rotation == 90.0 || rotation == 270.0);
    const int w_logical = sideways ? h_native : w_native;
    const int h_logical = sideways ? w_native : h_native;

    dst_.w = static_cast<float>(w_native);
    dst_.h = static_cast<float>(h_native);
    dst_.x = (w_logical - dst_.w) / 2.0f;
    dst_.y = (h_logical - dst_.h) / 2.0f;

    if (!SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error(std::string("SDL was not initialized. SDL error: ") + SDL_GetError());

    window_ = SDL_CreateWindow(title, w_logical * scale, h_logical * scale, SDL_WINDOW_RESIZABLE);
    renderer_ = window_ ? SDL_CreateRenderer(window_, nullptr) : nullptr;
    texture_ = renderer_ ? SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w_native, h_native) : nullptr;

    if (!window_ || !renderer_ || !texture_) {
        std::string msg = std::string("Window/renderer/texture not initialized. SDL error: ") + SDL_GetError();
        cleanup();
        throw std::runtime_error(msg);
    }

    SDL_SetWindowMinimumSize(window_, w_logical, h_logical);
    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderVSync(renderer_, 1);
    SDL_SetRenderLogicalPresentation(renderer_, w_logical, h_logical, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

Frontend::~Frontend() {
    cleanup();
}

bool Frontend::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            return false;

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE)
            return false;

        // send key press/release input to the input handler
        if ((event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) && input_handler_) {
            for (const KeyBinding& binding : KEY_BINDINGS) {
                if (binding.key == event.key.scancode)
                    input_handler_(binding.input, event.type == SDL_EVENT_KEY_DOWN);
            }
        }
    }

    return true;
}

void Frontend::present(const uint32_t* pixels) {
    SDL_UpdateTexture(texture_, nullptr, pixels, w_native_ * sizeof(uint32_t));

    SDL_RenderClear(renderer_);
    SDL_RenderTextureRotated(renderer_, texture_, nullptr, &dst_, rotation_, nullptr, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer_);
}

void Frontend::cleanup() {
    if (texture_) { SDL_DestroyTexture(texture_);   texture_ = nullptr; }
    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_);     window_ = nullptr; }
    SDL_Quit();
}
