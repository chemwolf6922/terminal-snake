#pragma once

#include <filesystem>
#include "Console.h"

namespace Snake
{
    class Utility
    {
    public:
        static std::filesystem::path GetSaveFileRoot();
        static void DrawBox(
            Console& console,
            size_t x_start, size_t y_start,
            size_t x_end, size_t y_end);
        static void DrawHorizontalLine(
            Console& console,
            size_t x_start, size_t y_start,
            size_t x_end,
            const std::string& character = "━");
    private:
        static std::filesystem::path _saveFileRoot;
    };
}
