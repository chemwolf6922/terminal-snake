#include <nlohmann/json.hpp>
#include <fstream>
#include "Settings.h"
#include "Utility.h"
#include "Constants.h"

using namespace Snake;

Settings Settings::Load()
{
    Settings settings{};
    auto path = GetFilePath();
    if (!std::filesystem::exists(path))
    {
        return settings;
    }
    std::ifstream file(path);
    if (file.fail())
    {
        throw std::runtime_error("Failed to open settings file");
    }
    nlohmann::json saved;
    file >> saved;
    file.close();
    if (!saved.is_object())
    {
        return settings;
    }
    if (saved.contains(USE_SIMPLE_GRAPHICS) && saved[USE_SIMPLE_GRAPHICS].is_boolean())
    {
        settings.useSimpleGraphics = saved[USE_SIMPLE_GRAPHICS].get<bool>();
    }
    if (saved.contains(GAME_SPEED) && saved[GAME_SPEED].is_number_integer())
    {
        int gameSpeedNumber = saved[GAME_SPEED].get<int>();
        if (gameSpeedNumber >= 0 && 
            gameSpeedNumber < static_cast<int>(GameSpeed::Count))
        {
            settings.gameSpeed = static_cast<GameSpeed>(gameSpeedNumber);
        }
    }
    return settings;
}

void Settings::Save() const
{
    nlohmann::json saved;
    saved[USE_SIMPLE_GRAPHICS] = useSimpleGraphics;
    saved[GAME_SPEED] = static_cast<int>(gameSpeed);
    auto path = GetFilePath();
    std::ofstream file(path);
    if (file.fail())
    {
        throw std::runtime_error("Failed to open settings file");
    }
    file << saved.dump(4);
    file.close();
}

std::filesystem::path Settings::GetFilePath()
{
    return Utility::GetSaveFileRoot() / Constants::SETTINGS_FILE;
}
