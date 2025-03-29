#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <filesystem>
#include <time.h>

namespace Snake
{
    class LeaderBoard
    {
    public:
        struct Score
        {
            std::string name;
            int score;
            time_t timestamp;
            bool operator>(const Score& other) const;
        };
        static void SaveScore(const std::string_view& name, int score);
        static std::vector<Score> LoadScores();
    private:
        static constexpr std::string_view KEY_NAME = "name";
        static constexpr std::string_view KEY_SCORE = "score";
        static constexpr std::string_view KEY_TIMESTAMP = "timestamp";

        static std::filesystem::path GetFilePath();
    };
}

