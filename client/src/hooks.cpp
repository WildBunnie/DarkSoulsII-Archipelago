#include "hooks.h"
#include "offsets.h"
#include <spdlog/spdlog.h>

extern Hooks* GameHooks;

// ============================= Utils =============================

bool showItem = true;

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

// function that allows us to get the itemLotId on giveItemsOnPickup
#ifdef _M_IX86
extern "C" int __cdecl getItemLotId(UINT_PTR thisPtr, UINT_PTR arg1, UINT_PTR arg2, UINT_PTR baseAddress);
#elif defined(_M_X64)
extern "C" int getItemLotId(UINT_PTR thisPtr, UINT_PTR arg1, UINT_PTR baseAddress);
#endif

// hooking this function to always start the game offline
// by blocking the game's dns lookup
typedef INT(__stdcall* getaddrinfo_t)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);

// fuction is called when player receives a reward (boss, covenant, npc or event)
typedef void(__thiscall* giveItemsOnReward_t)(UINT_PTR thisPtr, UINT_PTR pItemLot, INT idk1, INT idk2, INT idk3);
// fuction is called when player picks up an item
#ifdef _M_IX86
typedef void(__thiscall* giveItemsOnPickup_t)(UINT_PTR thisPtr, UINT_PTR idk1, UINT_PTR idk2);
#elif defined(_M_X64)
typedef void(__thiscall* giveItemsOnPickup_t)(UINT_PTR thisPtr, UINT_PTR idk1);
#endif


// this function adds the item to the players inventory
typedef char(__thiscall* addItemsToInventory_t)(UINT_PTR thisPtr, ItemStruct* itemsList, INT amountToGive, INT param_3);
// this function creates the structure that is passed to the function that displays the item popup
typedef void(__cdecl* createPopupStructure_t)(UINT_PTR displayStruct, ItemStruct* items, INT amountOfItems, INT displayMode);
// this function displays the item popup
typedef void(__thiscall* showItemPopup_t)(UINT_PTR thisPtr, UINT_PTR displayStruct);


getaddrinfo_t originalGetaddrinfo = nullptr;

giveItemsOnReward_t originalGiveItemsOnReward = nullptr;
giveItemsOnPickup_t originalGiveItemsOnPickup = nullptr;

addItemsToInventory_t originalAddItemsToInventory = nullptr;
createPopupStructure_t originalCreatePopupStructure = nullptr;
showItemPopup_t originalShowItemPopup = nullptr;

uintptr_t baseAddress;

uintptr_t Hooks::GetPointerAddress(uintptr_t gameBaseAddr, uintptr_t address, std::vector<uintptr_t> offsets)
{
    uintptr_t offset_null = NULL;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(gameBaseAddr + address), &offset_null, sizeof(offset_null), 0);
    uintptr_t pointeraddress = offset_null; // the address we need
    for (size_t i = 0; i < offsets.size() - 1; i++) // we dont want to change the last offset value so we do -1
    {
        ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pointeraddress + offsets.at(i)), &pointeraddress, sizeof(pointeraddress), 0);
    }
    return pointeraddress += offsets.at(offsets.size() - 1); // adding the last offset
}

// TODO: receive the other item information like amount, upgrades and infusions
void Hooks::giveItems(std::vector<int> ids) {

    if (ids.size() > 8) {
        return;
    }

    ItemStruct itemStruct;

    for (size_t i = 0; i < ids.size() && i < 8; ++i) {
        Item item;
        item.idk = 0;
        item.itemId = ids[i];
        item.durability = -1;
        item.amount = 1;
        item.upgrade = 0;
        item.infusion = 0;

        itemStruct.items[i] = item;
    }

    UINT_PTR displayStruct = (UINT_PTR)VirtualAlloc(nullptr, 0x110, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    originalAddItemsToInventory(GetPointerAddress(baseAddress, PointerOffsets::BaseA, PointerOffsets::AvailableItemBag), &itemStruct, ids.size(), 0);
    originalCreatePopupStructure(displayStruct, &itemStruct, ids.size(), 1);
    originalShowItemPopup(GetPointerAddress(baseAddress, PointerOffsets::BaseA, PointerOffsets::ItemGiveWindow), displayStruct);

    VirtualFree((LPVOID)displayStruct, 0, MEM_RELEASE);
}

// ============================= HOOKS =============================

INT __stdcall detourGetaddrinfo(PCSTR address, PCSTR port, const ADDRINFOA* pHints, PADDRINFOA* ppResult) {

    if (address && strcmp(address, GameHooks->addressToBlock) == 0) {
        return EAI_FAIL;
    }

    return originalGetaddrinfo(address, port, pHints, ppResult);
}

#ifdef _M_IX86
void __fastcall detourGiveItemsOnReward(UINT_PTR thisPtr, void* Unknown, UINT_PTR pItemLot, INT idk1, INT idk2, INT idk3) {
#elif defined(_M_X64)
void __cdecl detourGiveItemsOnReward(UINT_PTR thisPtr, UINT_PTR pItemLot, INT idk1, INT idk2, INT idk3) {
#endif

    int32_t itemLotId;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pItemLot), &itemLotId, sizeof(itemLotId), NULL);
    spdlog::debug("was rewarded: {}", itemLotId);

    if (GameHooks->locationsToCheck.contains(itemLotId)) {
        GameHooks->checkedLocations.push_back(itemLotId);
        GameHooks->locationsToCheck.erase(itemLotId);
        return;
    }

    return originalGiveItemsOnReward(thisPtr, pItemLot, idk1, idk2, idk3);
}

// this strategy with the booleans is not the best
// but if we dont call the original the item wont be removed from the map
bool giveNextItem = true;
bool showNextItem = true;
void detourGiveItemsOnPickupLogic(int32_t itemLotId){
    spdlog::debug("picked up: {}", itemLotId);

    if (itemLotId == -1) spdlog::warn("error finding out what itemLot was picked up");

    // 0 means its an item we dropped
    if (itemLotId != 0 && GameHooks->locationsToCheck.contains(itemLotId)) {
        GameHooks->checkedLocations.push_back(itemLotId);
        GameHooks->locationsToCheck.erase(itemLotId);
        giveNextItem = false;
        showNextItem = false;
    }
}

#ifdef _M_IX86
void __fastcall detourGiveItemsOnPickup(UINT_PTR thisPtr, void* Unknown, UINT_PTR idk1, UINT_PTR idk2) {
    int32_t itemLotId = getItemLotId(thisPtr, idk1, idk2, baseAddress);
    detourGiveItemsOnPickupLogic(itemLotId);
    return originalGiveItemsOnPickup(thisPtr, idk1, idk2);
}
#elif defined(_M_X64)
void __cdecl detourGiveItemsOnPickup(UINT_PTR thisPtr, UINT_PTR idk1) {
    int32_t itemLotId = getItemLotId(thisPtr, idk1, baseAddress);
    detourGiveItemsOnPickupLogic(itemLotId);
    return originalGiveItemsOnPickup(thisPtr, idk1);
}
#endif

#ifdef _M_IX86
char __fastcall detourAddItemsToInventory(UINT_PTR thisPtr, void* Unknown, ItemStruct* itemsList, INT amountToGive, INT param_3) {
#elif defined(_M_X64)
char __cdecl detourAddItemsToInventory(UINT_PTR thisPtr, ItemStruct* itemsList, INT amountToGive, INT param_3) {
#endif
    if (!giveNextItem) {
        giveNextItem = true;
        return 1;
    }

    return originalAddItemsToInventory(thisPtr, itemsList, amountToGive, param_3);
}

#ifdef _M_IX86
void __fastcall detourShowItemPopup(UINT_PTR thisPtr, void* Unknown, UINT_PTR displayStruct) {
#elif defined(_M_X64)
void __cdecl detourShowItemPopup(UINT_PTR thisPtr, UINT_PTR displayStruct) {
#endif
    if (!showNextItem) {
        showNextItem = true;
        return;
    }

    return originalShowItemPopup(thisPtr, displayStruct);
}

bool Hooks::initHooks() {

    HMODULE hModule = GetModuleHandleA("DarkSoulsII.exe");
    baseAddress = (uintptr_t)hModule;

    MH_Initialize();

    MH_CreateHookApi(L"ws2_32", "getaddrinfo", &detourGetaddrinfo, (LPVOID*)&originalGetaddrinfo);

    MH_CreateHook((LPVOID)(baseAddress + FunctionOffsets::GiveItemsOnReward), &detourGiveItemsOnReward, (LPVOID*)&originalGiveItemsOnReward);
    MH_CreateHook((LPVOID)(baseAddress + FunctionOffsets::GiveItemsOnPickup), &detourGiveItemsOnPickup, (LPVOID*)&originalGiveItemsOnPickup);
    
    MH_CreateHook((LPVOID)(baseAddress + FunctionOffsets::AddItemsToInventory), &detourAddItemsToInventory, (LPVOID*)&originalAddItemsToInventory);
    originalCreatePopupStructure = reinterpret_cast<createPopupStructure_t>(baseAddress + FunctionOffsets::CreatePopUpStruct);
    MH_CreateHook((LPVOID)(baseAddress + FunctionOffsets::ShowItemPopup), &detourShowItemPopup, (LPVOID*)&originalShowItemPopup);

    MH_EnableHook(MH_ALL_HOOKS);

    return true;
}

int prevHp, curHp = -1;
bool Hooks::playerJustDied() {
    prevHp = curHp;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)GetPointerAddress(baseAddress, PointerOffsets::BaseA, PointerOffsets::HP), &curHp, sizeof(int), NULL);
    if (prevHp != 0 && curHp == 0) {
        return true;
    }
    return false;
}

void Hooks::killPlayer() {
    int zeroHp = 0;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID*)GetPointerAddress(baseAddress, PointerOffsets::BaseA, PointerOffsets::HP), &zeroHp, sizeof(int), NULL);
}

bool Hooks::isPlayerInGame() {
    int value;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)GetPointerAddress(baseAddress, PointerOffsets::BaseA, PointerOffsets::PlayerCtrl), &value, sizeof(int), NULL);
    if (value != 0) {
        return true;
    }
    return false;
}