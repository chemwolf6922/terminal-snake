#include "SignalManager.h"
#include <sys/eventfd.h>
#include <csignal>
#include <functional>
#include <stdexcept>

using namespace Snake;

std::shared_ptr<SignalManager> SignalManager::_singleton{nullptr};

std::shared_ptr<SignalManager> SignalManager::GetSingleton(Tev& tev)
{
    if (_singleton == nullptr)
    {
        _singleton = std::shared_ptr<SignalManager>{new SignalManager{tev}};
    }
    return _singleton;
}

SignalManager::SignalManager(Tev& tev)
    : _tev(tev)
{
    _eventFd = eventfd(0, EFD_NONBLOCK);
    if (_eventFd == -1)
    {
        throw std::runtime_error("eventfd failed");
    }
    _readHandler = _tev.SetReadHandler(_eventFd, [this](){
        eventfd_t value = 0;
        int rc = eventfd_read(_eventFd, &value);
        if (rc != 0)
        {
            throw std::runtime_error("eventfd_read failed");
        }
        int signum = static_cast<int>(value);
        auto pair = _handlers.find(signum);
        if (pair != _handlers.end())
        {
            pair->second();
        }
    });
}

SignalManager::~SignalManager()
{
    Close();
}

void SignalManager::Close()
{
    for (auto handler : _handlers)
    {
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(handler.first, &sa, nullptr);
    }
    if (_eventFd != -1)
    {
        _readHandler.Clear();
        close(_eventFd);
        _eventFd = -1;
    }
    if (_singleton != nullptr)
    {
        _singleton.reset();
    }
}

void SignalManager::SetHandler(int signum, const std::function<void()>& handler)
{
    if (handler == nullptr)
    {
        _handlers.erase(signum);
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(signum, &sa, nullptr);
    }
    else
    {
        struct sigaction sa;
        sa.sa_handler = [](int signum){
            auto singleton = SignalManager::_singleton;
            if (singleton != nullptr)
            {
                eventfd_t value = static_cast<eventfd_t>(signum);
                eventfd_write(singleton->_eventFd, value);
            }
        };
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        int rc = sigaction(signum, &sa, nullptr);
        if (rc != 0)
        {
            throw std::runtime_error("sigaction failed");
        }
        _handlers[signum] = handler;
    }
}

