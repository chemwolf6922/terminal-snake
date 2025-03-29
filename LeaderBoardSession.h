#pragma once

#include "Session.h"
#include "Console.h"

namespace Snake
{
    class LeaderBoardSession : public Session<int, int>
    {
    public:
        LeaderBoardSession(Console& console);
        ~LeaderBoardSession() override;
        
        LeaderBoardSession(const LeaderBoardSession&) = delete;
        LeaderBoardSession& operator=(const LeaderBoardSession&) = delete;
        LeaderBoardSession(LeaderBoardSession&&) = delete;
        LeaderBoardSession& operator=(LeaderBoardSession&&) = delete;

        void Activate(const int& params) override;
        void Deactivate() override;
        void Close() override;

    private:
        static constexpr size_t LEFT_MARGIN = 10;
        static constexpr size_t SERIAL_LENGTH = 10;
        static constexpr size_t NAME_LENGTH = 10;
        static constexpr size_t SCORE_LENGTH = 10;
        static constexpr size_t TIME_LENGTH = 30;
        static constexpr size_t SERIAL_OFFSET = LEFT_MARGIN;
        static constexpr size_t NAME_OFFSET = SERIAL_OFFSET + SERIAL_LENGTH;
        static constexpr size_t SCORE_OFFSET = NAME_OFFSET + NAME_LENGTH;
        static constexpr size_t TIME_OFFSET = SCORE_OFFSET + SCORE_LENGTH;
        static constexpr size_t END_OFFSET = TIME_OFFSET + TIME_LENGTH;
        static constexpr size_t TOP_MARGIN = 5;

        Console& _console;
        bool _active{false};
        bool _closed{false};

        void ShowLeaderBoard();
    };
}
