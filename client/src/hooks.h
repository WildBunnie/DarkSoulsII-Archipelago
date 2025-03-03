#pragma once

#include <windows.h>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <WinSock2.h>
#include <winsock.h>
#include <Ws2tcpip.h>
#include <minhook.h>
#include <iostream>


struct locationReward {
    std::string item_name;
    std::string player_name;
};

class Hooks {
public:
    bool initHooks();
    void giveItems(std::vector<int> ids, bool onlyShow = false);
    void showLocationRewardMessage(int32_t locationId);
    void patchWeaponRequirements();

    bool playerJustDied();
    bool isPlayerInGame();
    void killPlayer();

    std::set<int64_t> locationsToCheck;
    std::list<int64_t> checkedLocations;
    std::map<int64_t, locationReward> locationRewards;
    bool isDeathLink;

#ifdef _M_IX86
    const char* addressToBlock = "frpg2-steam-ope.fromsoftware.jp";
#elif defined(_M_X64)
    const char* addressToBlock = "frpg2-steam64-ope-login.fromsoftware-game.net";
#endif
};