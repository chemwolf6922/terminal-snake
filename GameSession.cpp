#include <stdexcept>
#include "GameSession.h"
#include "Utility.h"

using namespace Snake;

GameSession::GameSession(Tev& tev, Console& console)
    : _tev(tev),
      _console(console),
      _score(console, 0, Constants::DISPLAY_HEIGHT - 1),
      _gameOverSession(tev, console)
{
}

GameSession::~GameSession()
{
    Close();
}

void GameSession::Activate(const GameSessionParams& params)
{
    if (_active || _closed)
    {
        return;
    }
    _active = true;
    _params = params;
    SetupGame();
}

void GameSession::SetupGame(bool reset)
{
    if (reset || _cells.empty())
    {
        /** Clear cells */
        _cells.clear();
        _emptyCells.Clear();
        for (int x = 0; x < _width; x++)
        {
            for (int y = 0; y < _height; y++)
            {
                _cells.push_back(CellType::Empty);
                _emptyCells.Insert({x, y});
            }
        }
        /** Put the snake starting from the 5th column of the middle row */
        _snake.clear();
        for (int i = 0; i < 4; i++)
        {
            int x = 5 + i;
            int y = _height/2;
            _snake.push_front({x, y});
            _cells[x + y*_width] = CellType::SnakeRight;
            _emptyCells.Remove({x, y});
        }
        _direction = Direction::Right;
        _pendingDirection = Direction::Right;
        /** reset score */
        _score = 0;
        /** Generate the initial food */
        _food = _emptyCells.PopRandom();
        _cells[_food.x + _food.y*_width] = CellType::Food;
    }
    _console.Clear();
    /** Draw border */
    Utility::DrawBox(
        _console,
        0, 0,
        Constants::DISPLAY_WIDTH - 1, Constants::DISPLAY_HEIGHT - 2);
    /** Draw snake */
    for (const auto& position : _snake)
    {
        auto cellType = _cells[position.x + position.y*_width];
        auto location = CellToLocation(position);
        auto character = CellTypeToChar(cellType, _params->useSimpleGraphics);
        _console.PutString(location.x, location.y, character);
    }
    /** Draw food */
    {
        auto location = CellToLocation(_food);
        auto character = CellTypeToChar(CellType::Food, _params->useSimpleGraphics);
        _console.PutString(location.x, location.y, character);
    }
    /** draw status bar */
    _score.ReDraw();
    /** Add input handlers */
    _console.SetKeyHandler(Console::EscapedKeys::Up, [this](){
        DirectionInputHandler(Direction::Up);
    });
    _console.SetKeyHandler(Console::EscapedKeys::Down, [this](){
        DirectionInputHandler(Direction::Down);
    });
    _console.SetKeyHandler(Console::EscapedKeys::Left, [this](){
        DirectionInputHandler(Direction::Left);
    });
    _console.SetKeyHandler(Console::EscapedKeys::Right, [this](){
        DirectionInputHandler(Direction::Right);
    });
    _console.SetKeyHandler('\x1b', [this](){
        SwitchBack({false});
    });
    /** start frame timer */
    _frameTimerHandle = _tev.SetTimeout(
        std::bind(&GameSession::FrameHandler, this),
        _params->frameTime);
}

void GameSession::Deactivate()
{
    if (!_active || _closed)
    {
        return;
    }
    _active = false;
    /** release frame timer */
    _tev.ClearTimeout(_frameTimerHandle);
    /** release input handlers */
    _console.SetKeyHandler('\x1b', nullptr);
    _console.SetKeyHandler(Console::EscapedKeys::Up, nullptr);
    _console.SetKeyHandler(Console::EscapedKeys::Down, nullptr);
    _console.SetKeyHandler(Console::EscapedKeys::Left, nullptr);
    _console.SetKeyHandler(Console::EscapedKeys::Right, nullptr);
}

void GameSession::Close()
{
    if (_closed)
    {
        return;
    }
    Deactivate();
    /** This MUST be set after deactivate */
    _closed = true;
    _gameOverSession.Close();
}

GameSession::Direction GameSession::OppositeDirection(const Direction& direction) const
{
    switch (direction)
    {
    case Direction::Up:
        return Direction::Down;
    case Direction::Down:
        return Direction::Up;
    case Direction::Left:
        return Direction::Right;
    case Direction::Right:
        return Direction::Left;
    default:
        throw std::invalid_argument("Invalid direction");
    }
}

void GameSession::FrameHandler()
{
    /** Set the next frame handler first */
    _frameTimerHandle = _tev.SetTimeout(
        std::bind(&GameSession::FrameHandler, this),
        _params->frameTime);
    auto head = _snake.front();
    auto previousHeadType = (_direction == Direction::Up) ? CellType::SnakeUp :
        (_direction == Direction::Down) ? CellType::SnakeDown :
        (_direction == Direction::Left) ? CellType::SnakeLeft :
        CellType::SnakeRight;
    _direction = _pendingDirection;
    auto previousHeadLocation = CellToLocation(head);
    Coordinate nextHead = head;
    switch (_direction)
    {
    case Direction::Up:
        nextHead.y = (head.y - 1 + _height) % _height;
        break;
    case Direction::Down:
        nextHead.y = (head.y + 1) % _height;
        break;
    case Direction::Left:
        nextHead.x = (head.x - 1 + _width) % _width;
        break;
    case Direction::Right:
        nextHead.x = (head.x + 1) % _width;
        break;
    default:
        throw std::invalid_argument("Invalid direction");
    }
    auto headLocation = CellToLocation(nextHead);
    auto nextHeadCellType = _cells[nextHead.x + nextHead.y*_width];
    bool generateFood = false;
    switch (nextHeadCellType)
    {
    case CellType::Empty: {
        auto tail = _snake.back();
        _cells[tail.x + tail.y*_width] = CellType::Empty;
        _emptyCells.Insert(tail);
        _snake.pop_back();
        auto emptyChar = CellTypeToChar(CellType::Empty, _params->useSimpleGraphics);
        auto tailLocation = CellToLocation(tail);
        _console.PutString(tailLocation.x, tailLocation.y, emptyChar);
        _emptyCells.Remove(nextHead);
    } break;
    case CellType::Food: {
        ++_score;
        generateFood = true;
    } break;
    default:
        GameOver({
            _score.GetScore(),
            previousHeadLocation.x,
            previousHeadLocation.y,
            CellTypeToChar(previousHeadType, _params->useSimpleGraphics)
        });
        return;
    }
    auto headType = (_direction == Direction::Up) ? CellType::SnakeUp :
        (_direction == Direction::Down) ? CellType::SnakeDown :
        (_direction == Direction::Left) ? CellType::SnakeLeft :
        CellType::SnakeRight;
    _cells[nextHead.x + nextHead.y*_width] = headType;
    _snake.push_front(nextHead);
    auto headChar = CellTypeToChar(headType, _params->useSimpleGraphics);
    _console.PutString(headLocation.x, headLocation.y, headChar);
    if (generateFood)
    {
        if (_emptyCells.Empty())
        {
            GameOver({
                _score.GetScore(),
                previousHeadLocation.x,
                previousHeadLocation.y,
                CellTypeToChar(previousHeadType, _params->useSimpleGraphics)
            });
            return;
        }
        _food = _emptyCells.PopRandom();
        _cells[_food.x + _food.y*_width] = CellType::Food;
        auto foodChar = CellTypeToChar(CellType::Food, _params->useSimpleGraphics);
        auto foodLocation = CellToLocation(_food);
        _console.PutString(foodLocation.x, foodLocation.y, foodChar);
    }
}

void GameSession::DirectionInputHandler(const Direction& direction)
{
    if (_direction == OppositeDirection(direction))
    {
        return;
    }
    _pendingDirection = direction;
}

std::string GameSession::CellTypeToChar(CellType cellType, bool simpleChar) const
{
    if (simpleChar)
    {
        switch (cellType)
        {
        case CellType::Empty:
            return "  ";
        case CellType::SnakeRight:
        case CellType::SnakeLeft:
        case CellType::SnakeUp:
        case CellType::SnakeDown:
            return "██";
        case CellType::Food:
            return "⚫";
        case CellType::Wall:
            return "▓▓";
        default:
            throw std::invalid_argument("Invalid cell type");
        }
    }
    else
    {
        switch (cellType)
        {
        case CellType::Empty:
            return "  ";
        case CellType::SnakeRight:
            return "⏩";
        case CellType::SnakeLeft:
            return "⏪";
        case CellType::SnakeUp:
            return "⏫";
        case CellType::SnakeDown:
            return "⏬";
        case CellType::Food:
            return "🍎";
        case CellType::Wall:
            return "▓▓";
        default:
            throw std::invalid_argument("Invalid cell type");
        }
    }
}

void GameSession::GameOver(const GameOverSessionParams& params)
{
    SwitchTo(
        _gameOverSession,
        params,
        std::function<void (const int&)>([this](const auto&){
            SwitchBack({true});
        }));
}

GameSession::Coordinate GameSession::CellToLocation(const Coordinate& cell) const
{
    return {cell.x*2 + 1, cell.y + 1};
}

bool GameSession::Coordinate::operator==(const Coordinate& other) const
{
    return x == other.x && y == other.y;
}

bool GameSession::Coordinate::operator<(const Coordinate& other) const
{
    return x < other.x || (x == other.x && y < other.y);
}

template <typename T>
void GameSession::RandomPool<T>::Insert(const T& value)
{
    if (_indexMap.find(value) != _indexMap.end())
    {
        return;
    }
    _pool.push_back(value);
    _indexMap[value] = _pool.size() - 1;
}

template <typename T>
void GameSession::RandomPool<T>::Remove(const T& value)
{
    auto it = _indexMap.find(value);
    if (it == _indexMap.end())
    {
        return;
    }
    size_t index = it->second;
    _indexMap.erase(it);
    auto tail = _pool.back();
    _pool.pop_back();
    _pool[index] = tail;
    _indexMap[tail] = index;
}

template <typename T>
T GameSession::RandomPool<T>::PopRandom()
{
    if (_pool.empty())
    {
        throw std::out_of_range("Empty pool");
    }
    std::uniform_int_distribution<size_t> dist(0, _pool.size() - 1);
    size_t index = dist(_rng);
    T value = _pool[index];
    Remove(value);
    return value;
}

template <typename T>
size_t GameSession::RandomPool<T>::Size() const
{
    return _pool.size();
}

template <typename T>
bool GameSession::RandomPool<T>::Empty() const
{
    return _pool.empty();
}

template <typename T>
void GameSession::RandomPool<T>::Clear()
{
    _pool.clear();
    _indexMap.clear();
}

GameSession::ScoreBar::ScoreBar(Console& console, int x, int y)
    : _console(console),
      _x(x),
      _y(y)
{
}

GameSession::ScoreBar& GameSession::ScoreBar::operator=(int score)
{
    _score = score;
    ReDraw();
    return *this;
}

GameSession::ScoreBar& GameSession::ScoreBar::operator++()
{
    _score++;
    ReDraw();
    return *this;
}

int GameSession::ScoreBar::GetScore() const
{
    return _score;
}

void GameSession::ScoreBar::ReDraw()
{
    std::string scoreStr = std::to_string(_score);
    scoreStr.append(4 - scoreStr.size(), ' ');
    _console.PutString(_x, _y, "Score: " + scoreStr);
}
