#include <tev-cpp/Tev.h>
#include "Session.h"
#include "Console.h"

namespace Snake
{
    struct GameOverSessionParams
    {
        int score{0};
        int headX{0};
        int headY{0};
        std::string headChar{""};
        bool useSimpleGraphics{false};
    };
    class GameOverSession : public Session<GameOverSessionParams, int>
    {
    public:
        GameOverSession(Tev& tev, Console& console);
        ~GameOverSession() override;

        GameOverSession(const GameOverSession& other) = delete;
        GameOverSession& operator=(const GameOverSession& other) = delete;
        GameOverSession(GameOverSession&& other) noexcept = delete;
        GameOverSession& operator=(GameOverSession&& other) noexcept = delete;

        void Activate(const GameOverSessionParams& params) override;
        void Deactivate() override;
        void Close() override;
    
    private:
        enum class AnimationState
        {
            Start,
            Alert1,
            Snake1,
            Alert2,
            End,
        };

        Tev& _tev;
        Console& _console;
        bool _active{false};
        bool _closed{false};
        GameOverSessionParams _params{};
        Tev::Timeout _animationTimer{};

        void PlayAnimation(AnimationState state);
        void ShowScoreDialog();
    };
}