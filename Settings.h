#pragma once

#include <filesystem>
#include <string_view>

namespace Snake
{
    struct Settings
    {
        enum class GameSpeed
        {
            Slow = 0,
            Normal = 1,
            Fast = 2,
            VeryFast = 3,
            Count = 4
        };
        bool useSimpleGraphics{false};
        GameSpeed gameSpeed{GameSpeed::Normal};
        static Settings Load();
        void Save() const;
    private:
        static constexpr std::string_view USE_SIMPLE_GRAPHICS = "useSimpleGraphics";
        static constexpr std::string_view GAME_SPEED = "gameSpeed";

        static std::filesystem::path GetFilePath();
    };
}
