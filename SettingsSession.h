#pragma once

#include <functional>
#include <memory>
#include <optional>
#include "Session.h"
#include "Console.h"
#include "Settings.h"

namespace Snake
{
    class SettingsSession : public Session<int, Settings>
    {
    public:
        SettingsSession(Console& console);
        ~SettingsSession() override;

        SettingsSession(const SettingsSession&) = delete;
        SettingsSession& operator=(const SettingsSession&) = delete;
        SettingsSession(SettingsSession&&) = delete;
        SettingsSession& operator=(SettingsSession&&) = delete;

        void Activate(const int& params) override;
        void Deactivate() override;
        void Close() override;

    private:
        class Menu
        {
        public:
            class BaseOption
            {
                friend class Menu;
            public:
                virtual ~BaseOption() = default;

            protected:
                virtual int Init(Console& console, size_t x, size_t y) = 0;
                virtual void SelectFirst() = 0;
                virtual bool SelectNext() = 0;
                virtual bool SelectPrevious() = 0;
                virtual void SelectLast() = 0;
                virtual void Deselect() = 0;
                virtual void Toggle() = 0;
            };

            class BoolOption : public BaseOption
            {
            public:
                BoolOption(const std::string_view& description, bool value, std::function<void(bool)> setter);
                ~BoolOption() override = default;

            protected:
                int Init(Console& console, size_t x, size_t y);
                void SelectFirst() override;
                bool SelectNext() override;
                bool SelectPrevious() override;
                void SelectLast() override;
                void Deselect() override;
                void Toggle() override;

            private:
                static constexpr std::string_view STR_ON = "[*]";
                static constexpr std::string_view STR_OFF = "[ ]";

                void Render();

                std::string _description;
                bool _value;
                std::function<void(bool)> _setter;
                bool _selected{false};
                Console* _console{nullptr};
                size_t _x{0};
                size_t _y{0};            
            };

            template <typename T>
            class EnumOption : public BaseOption
            {
            public:
                class SubOption
                {
                    friend class EnumOption;
                public:
                    SubOption(const std::string_view& description, const T& value);
                    ~SubOption() = default;

                private:
                    static constexpr std::string_view STR_ON = "(*)";
                    static constexpr std::string_view STR_OFF = "( )";

                    void Init(Console& console, size_t x, size_t y);
                    void Select();
                    void Deselect();
                    void ToggleOn();
                    void ToggleOff();
                    void Render();

                    std::string _description;
                    T _value;
                    bool _selected{false};
                    bool _on{false};
                    Console* _console{nullptr};
                    size_t _x{0};
                    size_t _y{0};
                };

                EnumOption(
                    const std::string_view& description,
                    const std::vector<SubOption>& options,
                    const T& value,
                    std::function<void(const T&)> setter);
                ~EnumOption() override = default;
            
            protected:
                int Init(Console& console, size_t x, size_t y);
                void SelectFirst() override;
                bool SelectNext() override;
                bool SelectPrevious() override;
                void SelectLast() override;
                void Deselect() override;
                void Toggle() override;

            private:
                class SelectedOption
                {
                public:
                    SelectedOption& operator=(const std::vector<SubOption>::iterator& option);
                    std::vector<SubOption>::iterator Get();
                    void Clear();
                private:
                    std::optional<typename std::vector<SubOption>::iterator> _option{};
                };

                class ToggledOption
                {
                public:
                    ToggledOption& operator=(const std::vector<SubOption>::iterator& option);
                    std::vector<SubOption>::iterator Get();
                    void Clear();
                private:
                    std::optional<typename std::vector<SubOption>::iterator> _option{};
                };

                std::string _description;
                std::vector<SubOption> _options;
                T _value;
                std::function<void(const T&)> _setter;
                SelectedOption _selectedOption{};
                ToggledOption _toggledOption{};
                Console* _console{nullptr};
                size_t _x{0};
                size_t _y{0};  
            };

            Menu(Console& console, size_t x, size_t y);
            ~Menu() = default;
            Menu(const Menu&) = delete;
            Menu& operator=(const Menu&) = delete;
            Menu(Menu&&) = delete;
            Menu& operator=(Menu&&) = delete;

            void AddOption(std::shared_ptr<BaseOption> option);
            void Clear();
            void BootStrap();
            void SelectNext();
            void SelectPrevious();
            void Toggle();

        private:
            class SelectedOption
            {
            public:
                void Set(const std::vector<std::shared_ptr<BaseOption>>::iterator& option, bool selectFirst = true);
                std::vector<std::shared_ptr<BaseOption>>::iterator Get();
                void Clear();
            private:
                std::optional<std::vector<std::shared_ptr<BaseOption>>::iterator> _option{};
            };

            Console& _console;
            size_t _x;
            size_t _y;
            std::vector<std::shared_ptr<BaseOption>> _options{};
            SelectedOption _selectedOption{};
        }; 

        Console& _console;
        Menu _menu;
        bool _active{false};
        bool _closed{false};
        Settings _settings{};
    };
}
