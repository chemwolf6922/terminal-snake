#include "LeaderBoardSession.h"
#include "LeaderBoard.h"
#include "Utility.h"
#include "Constants.h"

using namespace Snake;

LeaderBoardSession::LeaderBoardSession(Console& console)
    : _console(console)
{
}

LeaderBoardSession::~LeaderBoardSession()
{
    Close();
}

void LeaderBoardSession::Activate(const int& params)
{
    (void)params;
    if (_active || _closed)
    {
        return;
    }
    _active = true;
    _console.SetKeyHandler('\x1b', [this](){
        SwitchBack(0);
    });
    ShowLeaderBoard();
}

void LeaderBoardSession::Deactivate()
{
    if (!_active || _closed)
    {
        return;
    }
    _active = false;
    _console.SetKeyHandler('\x1b', nullptr);
}

void LeaderBoardSession::Close()
{
    if (_closed)
    {
        return;
    }
    Deactivate();
    /** This MUST be set after deactivate */
    _closed = true;
}

void LeaderBoardSession::ShowLeaderBoard()
{
    _console.Clear();
    size_t y = TOP_MARGIN;
    /** Draw boarder */
    Utility::DrawBox(
        _console,
        0, 0,
        Constants::DISPLAY_WIDTH - 1,
        Constants::DISPLAY_HEIGHT - 1);
    /** Show the sheet header */
    _console.PutString(SERIAL_OFFSET, y, "#");
    _console.PutString(NAME_OFFSET, y, "Name");
    _console.PutString(SCORE_OFFSET, y, "Score");
    _console.PutString(TIME_OFFSET, y, "Time");
    y++;
    /** Show the separate line */
    Utility::DrawHorizontalLine(
        _console,
        SERIAL_OFFSET, y++,
        END_OFFSET - 1);
    /** Load and display the leader board */
    auto scores = LeaderBoard::LoadScores();
    int i = 1;
    for (const auto& score : scores)
    {
        std::string serial = std::to_string(i++);
        std::string name = score.name;
        if (name.size() > NAME_LENGTH)
        {
            name.resize(NAME_LENGTH);
        }
        std::string scoreStr = std::to_string(score.score);
        std::string timeStr = std::ctime(&score.timestamp);
        _console.PutString(SERIAL_OFFSET, y, serial);
        _console.PutString(NAME_OFFSET, y, name);
        _console.PutString(SCORE_OFFSET, y, scoreStr);
        _console.PutString(TIME_OFFSET, y, timeStr);
        y++;
    }
}
