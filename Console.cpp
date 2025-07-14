#include "Console.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <memory>

using namespace Snake;

Console::Console(Tev& tev)
    : _tev(tev)
{
    /** Change to "Raw" mode */
    struct termios attr;
    int rc = tcgetattr(STDIN_FILENO, &attr);
    if (rc != 0)
    {
        throw std::runtime_error("tcgetattr failed");
    }
    attr.c_lflag &= ~(ICANON | ECHO);
    rc = tcsetattr(STDIN_FILENO, TCSANOW, &attr);
    if (rc != 0)
    {
        throw std::runtime_error("tcsetattr failed");
    }
    /** Hide the cursor */
    std::cout << "\x1b[?25l";
    std::flush(std::cout);
    /** Set stdin to non-blocking */
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
    {
        throw std::runtime_error("fcntl failed");
    }
    flags |= O_NONBLOCK;
    rc = fcntl(STDIN_FILENO, F_SETFL, flags);
    if (rc == -1)
    {
        throw std::runtime_error("fcntl failed");
    }
    /** Set stdin read handler */
    _readHandler = _tev.SetReadHandler(STDIN_FILENO, std::bind(&Console::TerminalKeyHandler, this));
}

Console::~Console()
{
    try
    {
        Close();
    }
    catch (...)
    {
    }
}

void Console::Close()
{
    if (_closed)
    {
        return;
    }
    _closed = true;
    /** Remove stdin read handler */
    _readHandler.reset();
    /** Reset cursor position and color */
    PutString(0, 0, "\x1b[0m");
    /** Clear screen */
    Clear();
    /** Show the cursor */
    std::cout << "\x1b[?25h";
    std::flush(std::cout);
    /** Change back to "cooked" mode */
    struct termios attr;
    int rc = tcgetattr(STDIN_FILENO, &attr);
    if (rc != 0)
    {
        throw std::runtime_error("tcgetattr failed");
    }
    attr.c_lflag |= (ICANON | ECHO);
    rc = tcsetattr(STDIN_FILENO, TCSANOW, &attr);
    if (rc != 0)
    {
        throw std::runtime_error("tcsetattr failed");
    }
}

void Console::SetErrorHandler(Console::ErrorHandler handler)
{
    _errorHandler = handler;
}

void Console::TerminalKeyHandler()
{
    uint8_t buffer[100];
    ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (bytesRead == 0)
    {
        Close();
        if (_errorHandler)
        {
            _errorHandler("EOF");
        }
        else
        {
            throw std::runtime_error("EOF");
        }
    }
    else if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }
        Close();
        auto errorString = strerror(errno);
        if (_errorHandler)
        {
            _errorHandler(errorString);
        }
        else
        {
            throw std::runtime_error(errorString);
        }
    }

    for (ssize_t i = 0; i < bytesRead; i++)
    {
        _inputBuffer.push_back(buffer[i]);
    }
    while(!_inputBuffer.empty())
    {
        std::string keySequence{};
        if (_inputBuffer[0] == '\x1b')
        {
            if (_inputBuffer.size() == 1 || _inputBuffer[1] != '[')
            {
                keySequence = "\x1b";
                _inputBuffer.pop_front();
            }
            else
            {
                /** escape sequence, find the end */
                ssize_t end = -1;
                for (size_t i = 2; i < _inputBuffer.size(); i++)
                {
                    if (_inputBuffer[i] >= 0x40 && _inputBuffer[i] <= 0x7E)
                    {
                        end = static_cast<ssize_t>(i);
                        break;
                    }
                }
                if (end == -1)
                {
                    /** incomplete */
                    break;
                }
                keySequence = std::string{_inputBuffer.begin(), _inputBuffer.begin() + end + 1};
                _inputBuffer.erase(_inputBuffer.begin(), _inputBuffer.begin() + end + 1);
            }
        }
        else
        {
            keySequence = std::string{_inputBuffer[0]};
            _inputBuffer.pop_front();
        }
        auto handler = _keyHandlers.find(keySequence);
        if (handler != _keyHandlers.end())
        {
            handler->second();
        }
    }
}

void Console::SetKeyHandler(char key, Console::KeyHandler handler)
{
    if (handler == nullptr)
    {
        _keyHandlers.erase(std::string{key});
        return;
    }
    _keyHandlers[std::string{key}] = handler;
}

void Console::SetKeyHandler(EscapedKeys key, Console::KeyHandler handler)
{
    std::string keySequence{};
    switch (key)
    {
    case EscapedKeys::Up:
        keySequence = "\x1b[A";
        break;
    case EscapedKeys::Down:
        keySequence = "\x1b[B";
        break;
    case EscapedKeys::Right:
        keySequence = "\x1b[C";
        break;
    case EscapedKeys::Left:
        keySequence = "\x1b[D";
        break;
    default:
        throw std::runtime_error("Unknown key");
    }
    if (handler == nullptr)
    {
        _keyHandlers.erase(keySequence);
        return;
    }
    _keyHandlers[keySequence] = handler;
}

void Console::TerminalStringHandler()
{
    uint8_t buffer[100];
    ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (bytesRead == 0)
    {
        Close();
        if (_errorHandler)
        {
            _errorHandler("EOF");
        }
        else
        {
            throw std::runtime_error("EOF");
        }
    }
    else if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }
        Close();
        auto errorString = strerror(errno);
        if (_errorHandler)
        {
            _errorHandler(errorString);
        }
        else
        {
            throw std::runtime_error(errorString);
        }
    }

    for (ssize_t i = 0; i < bytesRead; i++)
    {
        _inputBuffer.push_back(buffer[i]);
    }
    while(!_inputBuffer.empty())
    {
        char c = _inputBuffer[0];
        if (c == '\x1b')
        {
            if (_inputBuffer.size() == 1 || _inputBuffer[1] != '[')
            {
                _inputBuffer.pop_front();
            }
            else
            {
                /** escape sequence, find the end */
                ssize_t end = -1;
                for (size_t i = 2; i < _inputBuffer.size(); i++)
                {
                    if (_inputBuffer[i] >= 0x40 && _inputBuffer[i] <= 0x7E)
                    {
                        end = static_cast<ssize_t>(i);
                        break;
                    }
                }
                if (end == -1)
                {
                    /** incomplete */
                    break;
                }
                _inputBuffer.erase(_inputBuffer.begin(), _inputBuffer.begin() + end + 1);
            }
        }
        else if (c == '\n')
        {
            _inputBuffer.pop_front();
            /** input complete */
            _readHandler = _tev.SetReadHandler(STDIN_FILENO, std::bind(&Console::TerminalKeyHandler, this));
            /** hide the cursor */
            std::cout << "\x1b[?25l";
            std::flush(std::cout);
            auto handler = std::move(_stringHandler);
            _stringHandler = nullptr;
            handler(_inputString.input);
            return;
        }
        else if (c == '\x7F')
        {
            _inputBuffer.pop_front();
            /** backspace */
            if (!_inputString.input.empty())
            {
                _inputString.input.pop_back();
                std::cout << "\x1b[" << (_inputString.y + 1) << ";" << (_inputString.x + 1) << "H"
                    << _inputString.input << " ";
                std::cout << "\x1b[" << (_inputString.y + 1) << ";" << (_inputString.x + 1) << "H"
                    << _inputString.input;
                std::flush(std::cout);
            }
        }
        else if ((c < 0x20))
        {
            _inputBuffer.pop_front();
        }
        else if (_inputString.input.length() >= _inputString.maxLength)
        {
            _inputBuffer.pop_front();
        }
        else
        {
            _inputString.input.push_back(c);
            std::cout << c;
            std::flush(std::cout);
            _inputBuffer.pop_front();
        }
    }
}

void Console::GetString(size_t x, size_t y, size_t maxLength, Console::StringHandler handler)
{
    /** @todo Support non-ascii utf8 characters */
    if (handler == nullptr)
    {
        /** Exit getting string */
        _stringHandler = nullptr;
        _readHandler = _tev.SetReadHandler(STDIN_FILENO, std::bind(&Console::TerminalKeyHandler, this));
        /** Hide the cursor */
        std::cout << "\x1b[?25l";
        std::flush(std::cout);
        return;
    }
    _stringHandler = handler;
    _inputString.input.clear();
    _inputString.maxLength = maxLength;
    _inputString.x = x;
    _inputString.y = y;
    _readHandler = _tev.SetReadHandler(STDIN_FILENO, std::bind(&Console::TerminalStringHandler, this));

    std::cout << "\x1b[" << (y + 1) << ";" << (x + 1) << "H";
    /** show the cursor */
    std::cout << "\x1b[?25h";
    std::flush(std::cout);
}

void Console::Clear()
{
    std::cout << "\x1b[39m"
        << "\x1b[49m" 
        << "\x1b[2J";
    std::flush(std::cout);
}

void Console::PutString(
    size_t x, size_t y,
    const std::string_view& str,
    ForegroundColor foreGround,
    BackgroundColor backGround)
{
    std::cout << "\x1b[" << (y + 1) << ";" << (x + 1) << "H"
        << "\x1b[" << static_cast<int>(foreGround) << "m"
        << "\x1b[" << static_cast<int>(backGround) << "m"
        << str;
    std::flush(std::cout);
}
