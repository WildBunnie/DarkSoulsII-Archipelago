#pragma once

#include <windows.h>
#include <vector>
#include <list>
#include <set>
#include <WinSock2.h>
#include <winsock.h>
#include <Ws2tcpip.h>
#include <minhook.h>
#include <iostream>

class Hooks {
public:
    bool initHooks();
    uintptr_t GetPointerAddress(uintptr_t gameBaseAddr, uintptr_t address, std::vector<uintptr_t> offsets);
    void giveItems(std::vector<int> ids);

    bool playerJustDied();
    bool isPlayerInGame();
    void killPlayer();

    std::set<int64_t> locationsToCheck;
    std::list<int64_t> checkedLocations;
    bool isDeathLink;

#ifdef _M_IX86
    const char* addressToBlock = "frpg2-steam-ope.fromsoftware.jp";
#elif defined(_M_X64)
    const char* addressToBlock = "frpg2-steam64-ope-login.fromsoftware-game.net";
#endif
};