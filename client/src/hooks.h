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
    DWORD GetPointerAddress(DWORD gameBaseAddr, DWORD address, std::vector<DWORD> offsets);
    void giveItems(std::vector<int> ids);

    bool isPlayerDead();
    bool isPlayerInGame();
    void killPlayer();

    std::set<int64_t> locationsToCheck;
    std::list<int64_t> checkedLocations;
    bool isDeathLink;
};