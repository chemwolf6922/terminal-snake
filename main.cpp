#include <tev-cpp/Tev.h>
#include <iostream>
#include <signal.h>
#include "Console.h"
#include "MainSession.h"
#include "SignalManager.h"

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;

    Tev tev{};

    auto signalManager = Snake::SignalManager::GetSingleton(tev);

    Snake::Console console{tev};
    Snake::MainSession mainSession{tev, console};

    auto closeApp = [&](){
        mainSession.Close();
        console.Close();
        signalManager->Close();
    };
    signalManager->SetHandler(SIGINT, closeApp);
    signalManager->SetHandler(SIGTERM, closeApp);

    mainSession.Activate(0, [&](const int& result){
        (void)result;
        closeApp();
    });

    tev.MainLoop();

    return 0;
}
