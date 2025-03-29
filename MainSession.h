#include <tev-cpp/Tev.h>
#include <optional>
#include "Session.h"
#include "Console.h"
#include "GameSession.h"
#include "LeaderBoardSession.h"
#include "SettingsSession.h"
#include "Settings.h"

namespace Snake
{
    class MainSession : public Session<int, int>
    {
    public:
        MainSession(Tev& tev, Console& console);
        ~MainSession() override;

        MainSession(const MainSession& other) = delete;
        MainSession& operator=(const MainSession& other) = delete;
        MainSession(MainSession&& other) noexcept = delete;
        MainSession& operator=(MainSession&& other) noexcept = delete;

        using Session<int, int>::Activate;
        void Activate(const int& params) override;
        void Deactivate() override;
        void Close() override;
    private:
        class MainMenu
        {
        public:
            class Option
            {
            public:
                Option(
                    Console& console,
                    size_t x, size_t y, const std::string_view& text,
                    const std::function<void()>& action);
                void Select();
                void Deselect();
                std::function<void()> GetAction() const;
            private:
                Console& _console;
                size_t _x;
                size_t _y;
                std::string_view _text;
                std::function<void()> _action;
                bool _selected{false};
            };

            MainMenu(Console& console, size_t x, size_t y);
            void AddOption(const std::string_view& text, const std::function<void()>& action);
            void Bootstrap();
            void SelectNext();
            void SelectPrevious();
            void Confirm();
            void Clear();
        private:
            class SelectedOption
            {
            public:
                SelectedOption& operator=(const std::vector<Option>::iterator& option);
                std::vector<Option>::iterator Get();
                void Clear();
            private:
                std::optional<std::vector<Option>::iterator> _option;
            };

            Console& _console;
            size_t _x;
            size_t _y;
            std::vector<Option> _options{};
            SelectedOption _selectedOption{};
        };

        Tev& _tev;
        Console& _console;
        MainMenu _mainMenu;
        GameSession _gameSession;
        LeaderBoardSession _leaderBoardSession;
        SettingsSession _settingsSession;
        Settings _settings{};
        bool _active{false};
        bool _closed{false};
    };
}
