#include "SettingsSession.h"
#include "Utility.h"
#include "Constants.h"

using namespace Snake;

SettingsSession::SettingsSession(Console& console)
    : _console(console),
      _menu(console, 10, 2)
{
}

SettingsSession::~SettingsSession()
{
    Close();
}

void SettingsSession::Activate(const int& params)
{
    (void)params;
    if (_active || _closed)
    {
        return;
    }
    _active = true;
    _console.Clear();
    /** Draw boarder */
    Utility::DrawBox(
        _console,
        0, 0,
        Constants::DISPLAY_WIDTH - 1,
        Constants::DISPLAY_HEIGHT - 1);
    /** Init Menu */
    _settings = Settings::Load();
    _menu.AddOption(std::make_shared<SettingsSession::Menu::BoolOption>(
        "Use simple graphics",
        _settings.useSimpleGraphics,
        [this](bool value){
            _settings.useSimpleGraphics = value;
        }));
    _menu.AddOption(std::make_shared<SettingsSession::Menu::EnumOption<Settings::GameSpeed>>(
        "Game speed",
        std::vector<SettingsSession::Menu::EnumOption<Settings::GameSpeed>::SubOption>{
            {"Slow", Settings::GameSpeed::Slow},
            {"Normal", Settings::GameSpeed::Normal},
            {"Fast", Settings::GameSpeed::Fast},
            {"Very fast", Settings::GameSpeed::VeryFast}
        },
        _settings.gameSpeed,
        [this](const auto& value){
            _settings.gameSpeed = value;
        }
    ));
    _menu.BootStrap();
    _console.SetKeyHandler(Console::EscapedKeys::Up, [this](){
        _menu.SelectPrevious();
    });
    _console.SetKeyHandler(Console::EscapedKeys::Down, [this](){
        _menu.SelectNext();
    });
    _console.SetKeyHandler(' ', [this](){
        _menu.Toggle();
    });
    _console.SetKeyHandler('\x1b', [this](){
        SwitchBack(_settings);
    });
}

void SettingsSession::Deactivate()
{
    if (!_active || _closed)
    {
        return;
    }
    _active = false;
    _console.SetKeyHandler('\x1b', nullptr);
    _console.SetKeyHandler(Console::EscapedKeys::Up, nullptr);
    _console.SetKeyHandler(Console::EscapedKeys::Down, nullptr);
    _console.SetKeyHandler(' ', nullptr);
    _menu.Clear();
    _settings.Save();
}

void SettingsSession::Close()
{
    if (_closed)
    {
        return;
    }
    Deactivate();
    /** This MUST be set after deactivate */
    _closed = true;
}

SettingsSession::Menu::Menu(Console& console, size_t x, size_t y)
    : _console(console), _x(x), _y(y)
{
}

void SettingsSession::Menu::AddOption(std::shared_ptr<BaseOption> option)
{
    _options.emplace_back(std::move(option));
}

void SettingsSession::Menu::Clear()
{
    _selectedOption.Clear();
    _options.clear();
}

void SettingsSession::Menu::BootStrap()
{
    if (_options.empty())
    {
        return;
    }
    size_t y = _y;
    for (const auto& option : _options)
    {
        y += option->Init(_console, _x, y);
    }
    _selectedOption.Set(_options.begin(), true);
}

void SettingsSession::Menu::SelectNext()
{
    auto current = _selectedOption.Get();
    auto gotoNext = current->get()->SelectNext();
    if (gotoNext && current != _options.end() - 1)
    {
        _selectedOption.Set(current + 1, true);
    }
}

void SettingsSession::Menu::SelectPrevious()
{
    auto current = _selectedOption.Get();
    auto gotoPrevious = current->get()->SelectPrevious();
    if (gotoPrevious && current != _options.begin())
    {
        _selectedOption.Set(current - 1, false);
    }
}

void SettingsSession::Menu::Toggle()
{
    _selectedOption.Get()->get()->Toggle();
}

void SettingsSession::Menu::SelectedOption::Set(
    const std::vector<std::shared_ptr<SettingsSession::Menu::BaseOption>>::iterator& option, bool selectFirst)
{
    if (_option.has_value())
    {
        _option.value()->get()->Deselect();
    }
    _option = option;
    if (selectFirst)
    {
        _option.value()->get()->SelectFirst();
    }
    else
    {
        _option.value()->get()->SelectLast();
    }
}

std::vector<std::shared_ptr<SettingsSession::Menu::BaseOption>>::iterator SettingsSession::Menu::SelectedOption::Get()
{
    if (!_option.has_value())
    {
        throw std::runtime_error("No option selected");
    }
    return _option.value();
}

void SettingsSession::Menu::SelectedOption::Clear()
{
    if (_option.has_value())
    {
        _option.value()->get()->Deselect();
    }
    _option.reset();
}

SettingsSession::Menu::BoolOption::BoolOption(
    const std::string_view& description,
    bool value,
    std::function<void(bool)> setter)
    : _description(description),
      _value(value),
      _setter(setter)
{
}

int SettingsSession::Menu::BoolOption::Init(
    Console& console, size_t x, size_t y)
{
    _console = &console;
    _x = x;
    _y = y;
    Render();
    return 1;
}

void SettingsSession::Menu::BoolOption::SelectFirst()
{
    if (_selected)
    {
        return;
    }
    _selected = true;
    Render();
}

void SettingsSession::Menu::BoolOption::SelectLast()
{
    SelectFirst();
}

bool SettingsSession::Menu::BoolOption::SelectNext()
{
    return true;
}

bool SettingsSession::Menu::BoolOption::SelectPrevious()
{
    return true;
}

void SettingsSession::Menu::BoolOption::Deselect()
{
    if (!_selected)
    {
        return;
    }
    _selected = false;
    Render();
}

void SettingsSession::Menu::BoolOption::Toggle()
{
    _value = !_value;
    Render();
    _setter(_value);
}

void SettingsSession::Menu::BoolOption::Render()
{
    auto selector = std::string(_value ? STR_ON : STR_OFF);
    Console::ForegroundColor fg = _selected ?
        Console::ForegroundColor::Black : Console::ForegroundColor::Default;
    Console::BackgroundColor bg = _selected ?
        Console::BackgroundColor::White : Console::BackgroundColor::Default;
    _console->PutString(_x, _y, selector, fg, bg);
    _console->PutString(_x + selector.length(), _y, " " + _description);
}

template<typename T>
SettingsSession::Menu::EnumOption<T>::EnumOption(
    const std::string_view& description,
    const std::vector<SubOption>& options,
    const T& value,
    std::function<void(const T&)> setter)
    : _description(description),
      _options(options),
      _value(value),
      _setter(setter)
{
}

template<typename T>
int SettingsSession::Menu::EnumOption<T>::Init(
    Console& console, size_t x_in, size_t y_in)
{
    _console = &console;
    _x = x_in;
    _y = y_in;
    size_t y = _y;
    _console->PutString(_x, y++, "    " + _description);
    for (auto item = _options.begin(); item != _options.end(); item++)
    {
        item->Init(console, x_in + 4, y++);
        if (_value == item->_value)
        {
            _toggledOption = item;
        }
    }
    return y - _y;
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::SelectFirst()
{
    if (_options.empty())
    {
        return;
    }
    _selectedOption = _options.begin();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>:: SelectLast()
{
    if (_options.empty())
    {
        return;
    }
    _selectedOption = _options.end() - 1;
}

template<typename T>
bool SettingsSession::Menu::EnumOption<T>::SelectNext()
{
    auto current = _selectedOption.Get();
    if (current == _options.end() - 1)
    {
        return true;
    }
    _selectedOption = current + 1;
    return false;
}

template<typename T>
bool SettingsSession::Menu::EnumOption<T>::SelectPrevious()
{
    auto current = _selectedOption.Get();
    if (current == _options.begin())
    {
        return true;
    }
    _selectedOption = current - 1;
    return false;
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::Deselect()
{
    _selectedOption.Clear();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::Toggle()
{
    _toggledOption = _selectedOption.Get();
    _value = _toggledOption.Get()->_value;
    _setter(_value);
}

template<typename T>
SettingsSession::Menu::EnumOption<T>::SelectedOption& SettingsSession::Menu::EnumOption<T>::SelectedOption::operator=(
    const std::vector<SettingsSession::Menu::EnumOption<T>::SubOption>::iterator& option)
{
    if (_option.has_value())
    {
        _option.value()->Deselect();
    }
    _option = option;
    _option.value()->Select();
    return *this;
}

template<typename T>
std::vector<typename SettingsSession::Menu::EnumOption<T>::SubOption>::iterator SettingsSession::Menu::EnumOption<T>::SelectedOption::Get()
{
    if (!_option.has_value())
    {
        throw std::runtime_error("No option selected");
    }
    return _option.value();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::SelectedOption::Clear()
{
    if (_option.has_value())
    {
        _option.value()->Deselect();
    }
    _option.reset();
}

template<typename T>
SettingsSession::Menu::EnumOption<T>::ToggledOption& SettingsSession::Menu::EnumOption<T>::ToggledOption::operator=(
    const std::vector<SettingsSession::Menu::EnumOption<T>::SubOption>::iterator& option)
{
    if (_option.has_value())
    {
        _option.value()->ToggleOff();
    }
    _option = option;
    _option.value()->ToggleOn();
    return *this;
}

template<typename T>
std::vector<typename SettingsSession::Menu::EnumOption<T>::SubOption>::iterator SettingsSession::Menu::EnumOption<T>::ToggledOption::Get()
{
    if (!_option.has_value())
    {
        throw std::runtime_error("No option selected");
    }
    return _option.value();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::ToggledOption::Clear()
{
    /** The toggle stay on */
    _option.reset();
}

template<typename T>
SettingsSession::Menu::EnumOption<T>::SubOption::SubOption(
    const std::string_view& description, const T& value)
    : _description(description), _value(value)
{
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::SubOption::Init(
    Console& console, size_t x, size_t y)
{
    _console = &console;
    _x = x;
    _y = y;
    Render();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::SubOption::Select()
{
    if (_selected)
    {
        return;
    }
    _selected = true;
    Render();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::SubOption::Deselect()
{
    if (!_selected)
    {
        return;
    }
    _selected = false;
    Render();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::SubOption::ToggleOn()
{
    if (_on)
    {
        return;
    }
    _on = true;
    Render();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::SubOption::ToggleOff()
{
    if (!_on)
    {
        return;
    }
    _on = false;
    Render();
}

template<typename T>
void SettingsSession::Menu::EnumOption<T>::SubOption::Render()
{
    auto selector = std::string(_on ? STR_ON : STR_OFF);
    Console::ForegroundColor fg = _selected ?
        Console::ForegroundColor::Black : Console::ForegroundColor::Default;
    Console::BackgroundColor bg = _selected ?
        Console::BackgroundColor::White : Console::BackgroundColor::Default;
    _console->PutString(_x, _y, selector, fg, bg);
    _console->PutString(_x + selector.length(), _y, " " + _description);
}
