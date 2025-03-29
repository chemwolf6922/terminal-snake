#pragma once

namespace Snake
{
    namespace Constants
    {
        constexpr int DISPLAY_WIDTH = 80;
        constexpr int DISPLAY_HEIGHT = 25;
        constexpr std::string_view SAVE_FILE_ROOT = ".terminal_snake";
        constexpr std::string_view LEADER_BOARD_FILE = "leaderboard.json";
        constexpr int LEADER_BOARD_SIZE = 10;
        /** This is not a least upper bound */
        constexpr int SCORE_UPPER_BOUND = 99999;
        constexpr std::string_view SETTINGS_FILE = "settings.json";
    }
}
