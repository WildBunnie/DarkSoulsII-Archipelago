#pragma once

#include <windows.h>
#include <vector>
#include <list>
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
    void overwriteItemLots();

    bool isPlayerDead();
    bool isPlayerInGame();
    void killPlayer();

    void clearLocations(std::list<int64_t> locationsToRemove);

    std::list<int64_t> getLocations();
};