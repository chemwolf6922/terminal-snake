#include "Utility.h"
#include <pwd.h>
#include <unistd.h>
#include "Constants.h"

using namespace Snake;

std::filesystem::path Utility::_saveFileRoot{};

std::filesystem::path Utility::GetSaveFileRoot()
{
    if (_saveFileRoot.empty())
    {
        struct passwd* pw = getpwuid(getuid());
        if (!pw)
        {
            throw std::runtime_error("Failed to get user information");
        }
        auto homeDir = std::filesystem::path(pw->pw_dir);
        _saveFileRoot = homeDir / Constants::SAVE_FILE_ROOT;
        if (!std::filesystem::exists(_saveFileRoot))
        {
            std::filesystem::create_directories(_saveFileRoot);
        }
        _saveFileRoot = std::filesystem::canonical(_saveFileRoot);
    }
    return _saveFileRoot;
}

void Utility::DrawBox(
    Console& console,
    size_t x_start, size_t y_start,
    size_t x_end, size_t y_end)
{
    if (x_end - x_start < 2 ||
        y_end - y_start < 2)
    {
        throw std::invalid_argument("Invalid box size");
    }
    if (x_end >= Constants::DISPLAY_WIDTH ||
        y_end >= Constants::DISPLAY_HEIGHT)
    {
        throw std::out_of_range("Box coordinates out of range");
    }
    std::string border = "┏";
    for (size_t x = x_start + 1; x <= x_end - 1; x++)
    {
        border += "━";
    }
    border += "┓";
    console.PutString(x_start, y_start, border);
    for (size_t y = y_start + 1; y <= y_end - 1; y++)
    {
        console.PutString(x_start, y, "┃");
        console.PutString(x_end, y, "┃");
    }
    border = "┗";
    for (size_t x = x_start + 1; x <= x_end - 1; x++)
    {
        border += "━";
    }
    border += "┛";
    console.PutString(x_start, y_end, border);
}

void Utility::DrawHorizontalLine(
    Console& console,
    size_t x_start, size_t y_start,
    size_t x_end,
    const std::string& character)
{
    if (x_end - x_start < 1)
    {
        throw std::invalid_argument("Invalid line size");
    }
    if (x_end >= Constants::DISPLAY_WIDTH ||
        y_start >= Constants::DISPLAY_HEIGHT)
    {
        throw std::out_of_range("Line coordinates out of range");
    }
    std::string line{};
    for (size_t x = x_start; x <= x_end - 1; x++)
    {
        line += character;
    }
    console.PutString(x_start, y_start, line);
}
