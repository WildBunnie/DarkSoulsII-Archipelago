#ifndef HOOKS_H
#define HOOKS_H

#include <windows.h>
#include <vector>
#include <list>
#include <WinSock2.h>
#include <winsock.h>
#include <Ws2tcpip.h>

bool initHooks();
DWORD GetPointerAddress(DWORD gameBaseAddr, DWORD address, std::vector<DWORD> offsets);
void giveItems(std::vector<int> ids);
void overwriteItemLots();

std::list<int64_t> getLocations();
void clearLocations(std::list<int64_t> locationsToRemove);

struct Item {
    int idk;
    int itemId;
    int durability;
    short amount;
    char upgrade;
    char infusion;
};

struct ItemStruct {
    Item items[8];
};

// hooking this function to always start the game offline
// by blocking the game's dns lookup
typedef INT(__stdcall* getaddrinfo_t)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);

// fuction is called when an itemlot is picked up
typedef void(__thiscall* pickupItemLot_t)(UINT_PTR thisPtr, UINT_PTR idk);

// this function adds the item to the players inventory
typedef char(__thiscall* itemGive_t)(UINT_PTR thisPtr, ItemStruct* itemsList, INT amountToGive, INT param_3);

// this function creates the structure that is passed to the function that displays the item popup
typedef void(__cdecl* createPopupStructure_t)(UINT_PTR displayStruct, ItemStruct* items, INT amountOfItems, INT displayMode);

// this function displays the item popup
typedef void(__thiscall* showItemPopup_t)(UINT_PTR thisPtr, UINT_PTR displayStruct);

// used to check when player clicks the create new game button
typedef void(__thiscall* selectMenuOption_t)(UINT_PTR thisPtr);

// used to check when a player clicks to enter an existing save
typedef void(__thiscall* selectSaveSlot_t)(UINT_PTR thisPtr);

bool isPlayerDead();

void killPlayer();

#endif