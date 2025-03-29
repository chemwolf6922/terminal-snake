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
