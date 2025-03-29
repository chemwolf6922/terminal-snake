#pragma once

#include <tev-cpp/Tev.h>
#include <stdint.h>
#include <string>
#include <string_view>
#include <deque>
#include <functional>
#include <unordered_map>

namespace Snake
{
    class Console
    {
    public:
        enum class ForegroundColor
        {
            Black = 30,
            Red = 31,
            Green = 32,
            Yellow = 33,
            Blue = 34,
            Magenta = 35,
            Cyan = 36,
            White = 37,
            BrightBlack = 90,
            BrightRed = 91,
            BrightGreen = 92,
            BrightYellow = 93,
            BrightBlue = 94,
            BrightMagenta = 95,
            BrightCyan = 96,
            BrightWhite = 97,
            Default = 39,
        };

        enum class BackgroundColor
        {
            Black = 40,
            Red = 41,
            Green = 42,
            Yellow = 43,
            Blue = 44,
            Magenta = 45,
            Cyan = 46,
            White = 47,
            BrightBlack = 100,
            BrightRed = 101,
            BrightGreen = 102,
            BrightYellow = 103,
            BrightBlue = 104,
            BrightMagenta = 105,
            BrightCyan = 106,
            BrightWhite = 107,
            Default = 49,
        };

        enum class EscapedKeys
        {
            Up,
            Down,
            Left,
            Right,
        };

        typedef std::function<void()> KeyHandler;
        typedef std::function<void(const std::string_view&)> StringHandler;
        typedef std::function<void(const std::string_view&)> ErrorHandler;

        Console(Tev& tev);
        ~Console();

        Console(const Console& other) = delete;
        Console& operator=(const Console& other) = delete;
        Console(Console&& other) noexcept = delete;
        Console& operator=(Console&& other) noexcept = delete;

        void Close();
        void Clear();
        /**
         * @brief Put a string to the display.
         * 
         * @param x 
         * @param y 
         * @param str 
         */
        void PutString(
            size_t x, size_t y, 
            const std::string_view& str,
            ForegroundColor foreGround = ForegroundColor::Default,
            BackgroundColor backGround = BackgroundColor::Default);

        void SetKeyHandler(char key, KeyHandler handler);
        void SetKeyHandler(EscapedKeys key, KeyHandler handler);

        void GetString(size_t x, size_t y, size_t maxLength, StringHandler handler);

        void SetErrorHandler(ErrorHandler handler);
    private:
        struct StringInputState
        {
            std::string input{};
            size_t maxLength{0};
            size_t x;
            size_t y;
        }; 

        Tev& _tev;
        bool _closed{false};
        ErrorHandler _errorHandler{nullptr};
        std::unordered_map<std::string, KeyHandler> _keyHandlers{};
        StringHandler _stringHandler{nullptr};
        std::deque<char> _inputBuffer{};
        StringInputState _inputString{};

        void TerminalKeyHandler();
        void TerminalStringHandler();
    };
} // namespace Snake
