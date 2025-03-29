#pragma once

#include <filesystem>

namespace Snake
{
    class Utility
    {
    public:
        static std::filesystem::path GetSaveFileRoot();
    private:
        static std::filesystem::path _saveFileRoot;
    };
}
