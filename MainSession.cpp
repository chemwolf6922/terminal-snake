#include <stdexcept>
#include "MainSession.h"
#include "Utility.h"

using namespace Snake;

MainSession::MainSession(Tev& tev, Console& console)
    : _tev(tev),
      _console(console),
      _mainMenu(console, 30, 15),
      _gameSession(tev, console),
      _leaderBoardSession(console),
      _settingsSession(console)
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
    /** Load settings */
    _settings = Settings::Load();
    _console.Clear();
    /** Show banner */
    /** There is an additional \n at the start */
    constexpr std::string_view banner = R"(
                 _______..__   __.      ___       __  ___  _______ 
                /       ||  \ |  |     /   \     |  |/  / |   ____|
               |   (----`|   \|  |    /  ^  \    |  '  /  |  |__   
                \   \    |  . `  |   /  /_\  \   |    <   |   __|  
            .----)   |   |  |\   |  /  _____  \  |  .  \  |  |____ 
            |_______/    |__| \__| /__/     \__\ |__|\__\ |_______|)";
    _console.PutString(0,5,banner);
    /** Draw boarder */
    Utility::DrawBox(
        _console,
        0, 0,
        Constants::DISPLAY_WIDTH - 1,
        Constants::DISPLAY_HEIGHT - 1);
    /** Activate options */
    std::string startGameTitle = _resume ? 
        "[    Resume game   ]" : 
        "[    Start game    ]";
    _mainMenu.AddOption(startGameTitle, [this](){
        int frameTime = _settings.gameSpeed == Settings::GameSpeed::Slow ? 300 :
                       _settings.gameSpeed == Settings::GameSpeed::Normal ? 200 :
                       _settings.gameSpeed == Settings::GameSpeed::Fast ? 133 : 88;
        GameSessionParams params{
            frameTime,
            _settings.useSimpleGraphics,
            !_resume
        };
        SwitchTo(_gameSession, params, std::function<void(const GameSessionResult&)>(
            [this](const auto& result){
                _resume = !result.finished;
                Activate(0);
            }
        ));
    });
    _mainMenu.AddOption("[     Settings     ]", [this](){
        SwitchTo(_settingsSession, 0, std::function<void(const Settings&)>(
            [this](const auto& settings){
                _settings = settings;
                Activate(0);
            }
        ));
    });
    _mainMenu.AddOption("[    High scores   ]", [this](){
        SwitchTo(_leaderBoardSession, 0, std::function<void(const int&)>(
            [this](const auto&){
                Activate(0);
            }
        ));
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
    _leaderBoardSession.Close();
    _settingsSession.Close();
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
