#include "GameOverSession.h"
#include "LeaderBoard.h"

using namespace Snake;

GameOverSession::GameOverSession(Tev& tev, Console& console)
    : _tev(tev),
      _console(console)
{
}

GameOverSession::~GameOverSession()
{
    Close();
}

void GameOverSession::Activate(const GameOverSessionParams& params)
{
    if (_active || _closed)
    {
        return;
    }
    _active = true;
    _params = params;
    PlayAnimation(AnimationState::Start);
}

void GameOverSession::PlayAnimation(GameOverSession::AnimationState state)
{
    if (state != AnimationState::End)
    {
        _animationTimer = _tev.SetTimeout([=, this](){
            PlayAnimation(static_cast<AnimationState>(static_cast<int>(state) + 1));
        }, 500);
    }
    switch (state)
    {
    case AnimationState::Alert1:
    case AnimationState::Alert2:{
        std::string alertChar = _params.useSimpleGraphics ? "▓▓" : "⚠️";
        _console.PutString(_params.headX, _params.headY, alertChar);
    } break;
    case AnimationState::Snake1:
        _console.PutString(_params.headX, _params.headY, _params.headChar);
        break;
    case AnimationState::End:
        ShowScoreDialog();
        break;
    default:
        break;
    }
}

void GameOverSession::ShowScoreDialog()
{
    int x = 30;
    int y = 10;
    _console.PutString(x, y++, "┏━━━━━━━━━━━━━━━━━━┓");
    _console.PutString(x, y++, "┃    GAME  OVER    ┃");
    _console.PutString(x, y++, "┃  Score:          ┃");
    _console.PutString(x, y++, "┃  Name:           ┃");
    _console.PutString(x, y++, "┗━━━━━━━━━━━━━━━━━━┛");
    std::string scoreStr = std::to_string(_params.score);
    _console.PutString(x + 10, y - 3, scoreStr);
    _console.GetString(x + 10, y - 2, 9, [this](const std::string_view& name){
        LeaderBoard::SaveScore(name, _params.score);
        SwitchBack(0);
    });
}

void GameOverSession::Deactivate()
{
    if (!_active || _closed)
    {
        return;
    }
    _active = false;
    _console.GetString(0, 0, 0, nullptr);
    _animationTimer.Clear();
}

void GameOverSession::Close()
{
    if (_closed)
    {
        return;
    }
    Deactivate();
    _closed = true;
}


