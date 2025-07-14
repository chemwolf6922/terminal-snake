#pragma once

#include <tev-cpp/Tev.h>
#include <functional>
#include <unordered_map>
#include <memory>

namespace Snake
{
    class SignalManager
    {
    public:
        static std::shared_ptr<SignalManager> GetSingleton(Tev& tev);
        ~SignalManager();

        SignalManager(const SignalManager& other) = delete;
        SignalManager& operator=(const SignalManager& other) = delete;
        SignalManager(SignalManager&& other) noexcept = delete;
        SignalManager& operator=(SignalManager&& other) noexcept = delete;

        void Close();
        void SetHandler(int signum, const std::function<void()>& handler);

    private:
        static std::shared_ptr<SignalManager> _singleton;    

        Tev& _tev;
        int _eventFd = -1;
        std::unordered_map<int, std::function<void()>> _handlers{};
        Tev::FdHandler _readHandler{};
    
        SignalManager(Tev& tev);
    };
}
