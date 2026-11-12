#include "menu.hpp"
#include "asset_paths.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>

// Comments added with AI assistance to explain the purpose of each section of the code.

// ======================================================
// Rendering Helpers
// ======================================================
// Utility functions used only by menu.cpp for drawing and formatting menu text.
namespace fs = std::filesystem;

namespace
{
    void draw_text(
        SDL_Renderer* renderer,
        TTF_Font* font,
        const std::string& text,
        float x,
        float y,
        SDL_Color color)
    {
        if (!font)
        {
            return;
        }

        SDL_Surface* surface =
            TTF_RenderText_Solid(
                font,
                text.c_str(),
                text.length(),
                color);

        if (!surface)
        {
            return;
        }

        SDL_Texture* texture =
            SDL_CreateTextureFromSurface(
                renderer,
                surface);

        if (!texture)
        {
            SDL_DestroySurface(surface);
            return;
        }

        SDL_FRect dst =
        {
            x,
            y,
            static_cast<float>(surface->w),
            static_cast<float>(surface->h)
        };

        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &dst);

        SDL_DestroyTexture(texture);
        SDL_DestroySurface(surface);
    }

    void draw_text_centered(
        SDL_Renderer* renderer,
        TTF_Font* font,
        const std::string& text,
        float center_x,
        float y,
        SDL_Color color)
    {
        if (!font)
        {
            return;
        }

        int text_w = 0;
        int text_h = 0;

        TTF_GetStringSize(
            font,
            text.c_str(),
            text.length(),
            &text_w,
            &text_h);

        draw_text(
            renderer,
            font,
            text,
            center_x - text_w / 2.0f,
            y,
            color);
    }

    // Convert ROM directory names into a morereadable display format for the menu.
    // Example: lunar_rescue -> LUNAR RESCUE
    std::string format_game_name(
        const std::string& name)
    {
        std::string result = name;

        std::replace(
            result.begin(),
            result.end(),
            '_',
            ' ');

        for (char& c : result)
        {
            c = static_cast<char>(
                std::toupper(
                    static_cast<unsigned char>(c)));
        }

        return result;
    }
}

// ======================================================
// Menu Construction
// ======================================================

Menu::Menu()
    : selected_(0),
      game_selected_(false),
      font_(nullptr)
{
    discover_games();
}

Menu::~Menu()
{
    if (font_)
    {
        TTF_CloseFont(font_);
    }
}

void Menu::initialize()
{
    const fs::path font_path =
    asset_root() /
        "fonts" /
        "Roboto-Regular.ttf";

    font_ = TTF_OpenFont(
        font_path.string().c_str(),
        12);

    if (!font_)
    {
        std::cerr
            << "Failed to load font: "
            << SDL_GetError()
            << '\n';
    }
}

// ======================================================
// ROM Discovery
// ======================================================

void Menu::discover_games()
{
    games_.clear();

    const fs::path rom_root =
        asset_root() / "roms";

    if (!fs::exists(rom_root))
    {
        std::cerr
            << "ROM directory not found: "
            << rom_root
            << '\n';

        return;
    }

    for (const auto& entry :
         fs::directory_iterator(rom_root))
    {
        if (entry.is_directory())
        {
            games_.push_back(
                entry.path()
                    .filename()
                    .string());
        }
    }

    std::sort(
        games_.begin(),
        games_.end());
}

// ======================================================
// Input
// ======================================================

void Menu::handle_input(
    const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
    {
        return;
    }

    switch (event.key.scancode)
    {
        case SDL_SCANCODE_UP:

            if (selected_ > 0)
            {
                --selected_;
            }

            break;

        case SDL_SCANCODE_DOWN:

            if (selected_ <
                static_cast<int>(games_.size()) - 1)
            {
                ++selected_;
            }

            break;

        case SDL_SCANCODE_RETURN:

            if (!games_.empty())
            {
                game_selected_ = true;
            }

            break;

        default:
            break;
    }
}

// ======================================================
// Rendering
// ======================================================

void Menu::render(SDL_Renderer* renderer)
{
    if (!font_)
    {
        return;
    }

    SDL_Color cyan =
    {
        0, 255, 255, 255
    };

    SDL_Color green =
    {
        0, 255, 0, 255
    };

    SDL_Color white =
    {
        255, 255, 255, 255
    };

    SDL_Color gray =
    {
        180, 180, 180, 255
    };

    constexpr float BORDER_X = 12.0f;
    constexpr float BORDER_Y = 12.0f;
    constexpr float BORDER_W = 200.0f;
    constexpr float BORDER_H = 230.0f;

    SDL_FRect border =
    {
        BORDER_X,
        BORDER_Y,
        BORDER_W,
        BORDER_H
    };

    const float center_x =
        BORDER_X + BORDER_W / 2.0f;

    SDL_SetRenderDrawColor(
        renderer,
        0,
        255,
        0,
        255);

    SDL_RenderRect(
        renderer,
        &border);

    // --------------------------------------------------
    // Title
    // --------------------------------------------------

    draw_text_centered(
        renderer,
        font_,
        "8080 EMULATOR",
        center_x,
        20.0f,
        cyan);

    draw_text_centered(
        renderer,
        font_,
        "SELECT GAME",
        center_x,
        50.0f,
        white);

    if (games_.empty())
    {
        draw_text_centered(
            renderer,
            font_,
            "NO ROMS FOUND",
            center_x,
            120.0f,
            green);

        return;
    }

    const Uint64 ticks = SDL_GetTicks();

    const float arrow_offset =
        ((ticks / 350) % 2 == 0)
        ? 0.0f
        : 2.0f;

    constexpr float ARROW_TOP_Y = 80.0f;
    constexpr float PREVIOUS_Y  = 100.0f;
    constexpr float CURRENT_Y   = 130.0f;
    constexpr float NEXT_Y      = 160.0f;
    constexpr float ARROW_BOT_Y = 180.0f;

    // --------------------------------------------------
    // Up arrow
    // --------------------------------------------------

    if (selected_ > 0)
    {
        draw_text_centered(
            renderer,
            font_,
            "^",
            center_x,
            ARROW_TOP_Y - arrow_offset,
            green);
    }

    // --------------------------------------------------
    // Previous game
    // --------------------------------------------------

    if (selected_ > 0)
    {
        draw_text_centered(
            renderer,
            font_,
            format_game_name(
                games_[selected_ - 1]),
            center_x,
            PREVIOUS_Y,
            gray);
    }

    // --------------------------------------------------
    // Current selection
    // --------------------------------------------------

    SDL_FRect selection_box =
    {
        center_x - 70.0f,
        CURRENT_Y - 2.0f,
        140.0f,
        20.0f
    };

    SDL_SetRenderDrawColor(
        renderer,
        0,
        120,
        0,
        255);

    SDL_RenderFillRect(
        renderer,
        &selection_box);

    SDL_SetRenderDrawColor(
        renderer,
        0,
        255,
        0,
        255);

    SDL_RenderRect(
        renderer,
        &selection_box);

    draw_text_centered(
        renderer,
        font_,
        format_game_name(
            games_[selected_]),
        center_x,
        CURRENT_Y,
        white);

    // --------------------------------------------------
    // Next game
    // --------------------------------------------------

    if (selected_ <
        static_cast<int>(games_.size()) - 1)
    {
        draw_text_centered(
            renderer,
            font_,
            format_game_name(
                games_[selected_ + 1]),
            center_x,
            NEXT_Y,
            gray);
    }

    // --------------------------------------------------
    // Down arrow
    // --------------------------------------------------

    if (selected_ <
        static_cast<int>(games_.size()) - 1)
    {
        draw_text_centered(
            renderer,
            font_,
            "v",
            center_x,
            ARROW_BOT_Y + arrow_offset,
            green);
    }

    // --------------------------------------------------
    // Footer
    // --------------------------------------------------


    if ((ticks / 500) % 2 == 0)
    {
        draw_text_centered(
            renderer,
            font_,
            "PRESS ENTER",
            center_x,
            210.0f,
            green);
    }
}

// ======================================================
// Accessors
// ======================================================

const std::vector<std::string>&
Menu::games() const
{
    return games_;
}

std::string Menu::selected_game() const
{
    if (games_.empty())
    {
        return "";
    }

    return games_[selected_];
}

bool Menu::game_selected() const
{
    return game_selected_;
}
