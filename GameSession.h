#include <tev-cpp/Tev.h>
#include <deque>
#include <map>
#include <optional>
#include <random>
#include "Session.h"
#include "Console.h"
#include "Constants.h"
#include "GameOverSession.h"

namespace Snake
{
    struct GameSessionParams
    {
        int frameTime{1000};
        bool useSimpleGraphics{false};
        bool newGame{true};
    };
    struct GameSessionResult
    {
        bool finished{false};
    };
    class GameSession : public Session<GameSessionParams, GameSessionResult>
    {
    public:
        GameSession(Tev& tev, Console& console);
        ~GameSession() override;

        GameSession(const GameSession& other) = delete;
        GameSession& operator=(const GameSession& other) = delete;
        GameSession(GameSession&& other) noexcept = delete;
        GameSession& operator=(GameSession&& other) noexcept = delete;

        void Activate(const GameSessionParams& params) override;
        void Deactivate() override;
        void Close() override;
    
    private:
        struct Coordinate
        {
            int x{0};
            int y{0};
            bool operator==(const Coordinate& other) const;
            bool operator<(const Coordinate& other) const;
        };
        enum class CellType
        {
            Empty,
            SnakeRight,
            SnakeLeft,
            SnakeUp,
            SnakeDown,
            Food,
            Wall,
        };
        enum class Direction
        {
            Up,
            Down,
            Left,
            Right,
        };
        template <typename T>
        class RandomPool
        {
        public:
            RandomPool() = default;
            ~RandomPool() = default;
            void Insert(const T& value);
            void Remove(const T& value);
            T PopRandom();
            size_t Size() const;
            bool Empty() const;
            void Clear();
        private:
            std::mt19937 _rng{std::random_device{}()};
            std::vector<T> _pool{};
            std::map<T, size_t> _indexMap{};
        };
        class ScoreBar
        {
        public:
            ScoreBar(Console& console, int x, int y);
            ~ScoreBar() = default;
            ScoreBar& operator=(int score);
            ScoreBar& operator++();
            int GetScore() const;
            void ReDraw();
        private:
            Console& _console;
            int _x;
            int _y;
            int _score{0};
        };

        /** -3 borders + status bar */
        static constexpr int _height{Constants::DISPLAY_HEIGHT - 3};
        /** -2 borders, /2 double width cell */
        static constexpr int _width{(Constants::DISPLAY_WIDTH - 2)/2};

        Tev& _tev;
        Console& _console;
        ScoreBar _score;
        GameOverSession _gameOverSession;
        bool _active{false};
        bool _closed{false};
        bool _finished{false};
        std::vector<CellType> _cells{};
        std::deque<Coordinate> _snake{};
        Coordinate _food{};
        RandomPool<Coordinate> _emptyCells{};
        Direction _direction{Direction::Right};
        Direction _pendingDirection{Direction::Right};
        std::optional<Tev::Timeout> _frameTimerHandle{std::nullopt};
        std::optional<GameSessionParams> _params{};

        void SetupGame(bool reset = true);
        Direction OppositeDirection(const Direction& direction) const;
        void FrameHandler();
        void DirectionInputHandler(const Direction& direction);
        std::string CellTypeToChar(CellType cellType, bool simpleChar) const;
        Coordinate CellToLocation(const Coordinate& cellCoordinate) const;
        void GameOver(const GameOverSessionParams& params);
    };
}
