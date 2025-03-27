#include "MainSession.h"

#include <stdexcept>

using namespace Snake;

MainSession::MainSession(Tev& tev, Console& console)
    : _tev(tev),
      _console(console),
      _mainMenu(console, 30, 15),
      _gameSession(tev, console)
{
}

MainSession::~MainSession()
{
    Close();
}

void MainSession::Activate(const int& params)
{
    (void)params;
    if (_active || _closed)
    {
        return;
    }
    _active = true;
    /** Show banner */
    _console.Clear();
    /** There is an additional \n at the start */
    constexpr std::string_view banner = R"(
                 _______..__   __.      ___       __  ___  _______ 
                /       ||  \ |  |     /   \     |  |/  / |   ____|
               |   (----`|   \|  |    /  ^  \    |  '  /  |  |__   
                \   \    |  . `  |   /  /_\  \   |    <   |   __|  
            .----)   |   |  |\   |  /  _____  \  |  .  \  |  |____ 
            |_______/    |__| \__| /__/     \__\ |__|\__\ |_______|)";
    _console.PutString(0,5,banner);
    /** Activate options */
    _mainMenu.AddOption("[    Start game    ]", [this](){
        /** @todo get frame time from settings */
        GameSessionParams params{300};
        SwitchTo(_gameSession, params, std::function<void(const GameSessionResult&)>(
            [this](const GameSessionResult& result){
                (void)result;
                Activate(0);
                /** @todo if the game did not finish, resume instead of starting a new one */
            }
        ));
    });
    _mainMenu.AddOption("[     Settings     ]", [this](){
        /** @todo */
    });
    _mainMenu.AddOption("[    High scores   ]", [this](){
        /** @todo */
    });
    _mainMenu.AddOption("[       Exit       ]", [this](){
        SwitchBack(0);
    });
    _console.SetKeyHandler(Console::EscapedKeys::Up, [this](){
        _mainMenu.SelectPrevious();
    });
    _console.SetKeyHandler(Console::EscapedKeys::Down, [this](){
        _mainMenu.SelectNext();
    });
    _console.SetKeyHandler('\n', [this](){
        _mainMenu.Confirm();
    });
    _console.SetKeyHandler(' ', [this](){
        _mainMenu.Confirm();
    });
    _mainMenu.Bootstrap();
}

void MainSession::Deactivate()
{
    if (!_active || _closed)
    {
        return;
    }
    _active = false;
    _console.SetKeyHandler(Console::EscapedKeys::Up, nullptr);
    _console.SetKeyHandler(Console::EscapedKeys::Down, nullptr);
    _console.SetKeyHandler('\n', nullptr);
    _console.SetKeyHandler(' ', nullptr);
    _mainMenu.Clear();
}

void MainSession::Close()
{
    if (_closed)
    {
        return;
    }
    Deactivate();
    /** This MUST be set after deactivate */
    _closed = true;
    /** propagate close */
    _gameSession.Close();
    /** @todo */
}

MainSession::MainMenu::Option::Option(
    Console& console,
    size_t x, size_t y, const std::string_view& text,
    const std::function<void()>& action)
    : _console(console), _x(x), _y(y), _text(text), _action(action)
{
}

void MainSession::MainMenu::Option::Select()
{
    _selected = true;
    _console.PutString(_x, _y, _text, Console::ForegroundColor::Black, Console::BackgroundColor::White);
}

void MainSession::MainMenu::Option::Deselect()
{
    _selected = false;
    _console.PutString(_x, _y, _text);
}

std::function<void()> MainSession::MainMenu::Option::GetAction() const
{
    return _action;
}

MainSession::MainMenu::SelectedOption& MainSession::MainMenu::SelectedOption::operator=(
    const std::vector<Option>::iterator& option)
{
    if (_option.has_value())
    {
        _option.value()->Deselect();
    }
    _option = option;
    _option.value()->Select();
    return *this;
}

std::vector<MainSession::MainMenu::Option>::iterator MainSession::MainMenu::SelectedOption::Get()
{
    if (!_option.has_value())
    {
        throw std::runtime_error("No option selected");
    }
    return _option.value();
}

void MainSession::MainMenu::SelectedOption::Clear()
{
    _option.reset();
}

MainSession::MainMenu::MainMenu(Console& console, size_t x, size_t y)
    : _console(console), _x(x), _y(y)
{
}

void MainSession::MainMenu::AddOption(const std::string_view& text, const std::function<void()>& action)
{
    size_t y = _y + _options.size();
    _options.emplace_back(_console, _x, y, text, action);
}

void MainSession::MainMenu::Bootstrap()
{
    if (_options.empty())
    {
        return;
    }
    _selectedOption = _options.begin();
    if (_options.size() == 1)
    {
        return;
    }
    for (auto option = _options.begin() + 1; option != _options.end(); option++)
    {
        option->Deselect();
    }
}

void MainSession::MainMenu::SelectNext()
{
    auto current = _selectedOption.Get();
    if (current == _options.end() - 1)
    {
        return;
    }
    _selectedOption = current + 1;
}

void MainSession::MainMenu::SelectPrevious()
{
    auto current = _selectedOption.Get();
    if (current == _options.begin())
    {
        return;
    }
    _selectedOption = current - 1;
}

void MainSession::MainMenu::Confirm()
{
    auto current = _selectedOption.Get();
    auto action = current->GetAction();
    action();
}

void MainSession::MainMenu::Clear()
{
    _selectedOption.Clear();
    _options.clear();
}
