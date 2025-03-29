#include "LeaderBoard.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include "Constants.h"
#include "Utility.h"

using namespace Snake;

std::filesystem::path LeaderBoard::GetFilePath()
{
    return Utility::GetSaveFileRoot() / Constants::LEADER_BOARD_FILE;
}

bool LeaderBoard::Score::operator>(const Score& other) const
{
    if (score != other.score)
    {
        return score > other.score;
    }
    if (timestamp != other.timestamp)
    {
        /** Older score is considred higher */
        return timestamp < other.timestamp;
    }
    return name > other.name;
}

void LeaderBoard::SaveScore(const std::string_view& name, int scoreNum)
{
    time_t now = time(nullptr);
    std::string nameStr(name);
    if (nameStr.empty())
    {
        nameStr = "Anonymous";
    }
    Score score{ nameStr, scoreNum, now };
    std::vector<Score> scores = LoadScores();
    scores.push_back(score);
    std::sort(scores.begin(), scores.end(), std::greater<Score>());
    if (scores.size() > Constants::LEADER_BOARD_SIZE)
    {
        scores.resize(Constants::LEADER_BOARD_SIZE);
    }
    nlohmann::json scoresJson = nlohmann::json::array();
    for (const auto& s : scores)
    {
        scoresJson.push_back({
            {KEY_NAME, s.name},
            {KEY_SCORE, s.score},
            {KEY_TIMESTAMP, s.timestamp}
        });
    }
    std::ofstream file(GetFilePath());
    if (file.fail())
    {
        throw std::runtime_error("Failed to open leaderboard file for writing");
    }
    file << scoresJson.dump(4);
    file.close();
}

std::vector<LeaderBoard::Score> LeaderBoard::LoadScores()
{
    auto filePath = GetFilePath();
    if (!std::filesystem::exists(filePath))
    {
        return {};
    }
    std::ifstream file(GetFilePath());
    if (file.fail())
    {
        throw std::runtime_error("Failed to open leaderboard file for reading");
    }
    nlohmann::json scoresJson;
    file >> scoresJson;
    file.close();
    if (!scoresJson.is_array())
    {
        throw std::runtime_error("Invalid leaderboard file format");
    }
    std::vector<Score> scores;
    for (const auto& item : scoresJson)
    {
        if (!(item.contains(KEY_NAME) && item[KEY_NAME].is_string())
            || !(item.contains(KEY_SCORE) && item[KEY_SCORE].is_number())
            || !(item.contains(KEY_TIMESTAMP) && item[KEY_TIMESTAMP].is_number()))
        {
            /** Ignore invalid values */
            continue;
        }
        int score = item[KEY_SCORE].get<int>();
        if (score < 0 || score > Constants::SCORE_UPPER_BOUND)
        {
            /** Ignore invalid scores */
            continue;
        }
        scores.emplace_back(item[KEY_NAME].get<std::string>(),
                            score,
                            item[KEY_TIMESTAMP].get<time_t>());
    }
    std::sort(scores.begin(), scores.end(), std::greater<Score>());
    return scores;
}
