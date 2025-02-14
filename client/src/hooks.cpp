#include "hooks.h"
#include "FunctionOffsets.h"
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

// hooking this function to always start the game offline
// by blocking the game's dns lookup
typedef INT(__stdcall* getaddrinfo_t)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);

// fuction is called when an itemlot is picked up
typedef void(__thiscall* pickupItemLot_t)(UINT_PTR thisPtr, UINT_PTR idk);

// fuction is called when player receives a reward (boss, covenant, npc or event)
typedef void(__thiscall* giveReward_t)(UINT_PTR thisPtr, UINT_PTR* pItemLot, INT idk1, INT idk2, INT idk3);

// this function adds the item to the players inventory
typedef char(__thiscall* addToInventory_t)(UINT_PTR thisPtr, ItemStruct* itemsList, INT amountToGive, INT param_3);

// this function creates the structure that is passed to the function that displays the item popup
typedef void(__cdecl* createPopupStructure_t)(UINT_PTR displayStruct, ItemStruct* items, INT amountOfItems, INT displayMode);

// this function displays the item popup
typedef void(__thiscall* showItemPopup_t)(UINT_PTR thisPtr, UINT_PTR displayStruct);

// used to check when player clicks the create new game button
typedef void(__thiscall* selectMenuOption_t)(UINT_PTR thisPtr);

// used to check when a player clicks to enter an existing save
typedef void(__thiscall* selectSaveSlot_t)(UINT_PTR thisPtr);

getaddrinfo_t originalGetaddrinfo = nullptr;
pickupItemLot_t originalPickupItemLot = nullptr;
giveReward_t originalGiveReward = nullptr;
addToInventory_t originalAddToInventory = nullptr;
createPopupStructure_t originalCreatePopupStructure = nullptr;
showItemPopup_t originalShowItemPopup = nullptr;
selectMenuOption_t originalSelectMenuOption = nullptr;
selectSaveSlot_t originalSelectSaveSlot = nullptr;

DWORD baseAddress;

DWORD Hooks::GetPointerAddress(DWORD gameBaseAddr, DWORD address, std::vector<DWORD> offsets)
{
    DWORD offset_null = NULL;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(gameBaseAddr + address), &offset_null, sizeof(offset_null), 0);
    DWORD pointeraddress = offset_null; // the address we need
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

    originalAddToInventory(GetPointerAddress(baseAddress, 0x1150414, { 0x60, 0x8, 0x8, 0x0 }), &itemStruct, ids.size(), 0);
    originalCreatePopupStructure(displayStruct, &itemStruct, ids.size(), 1);
    originalShowItemPopup(GetPointerAddress(baseAddress, 0x1150414, { 0xCC4, 0x0 }), displayStruct);

    VirtualFree((LPVOID)displayStruct, 0, MEM_RELEASE);
}

void Hooks::overwriteItemLots() {

    DWORD itemLotsAddress = GetPointerAddress(baseAddress, 0x1150414, { 0x60, 0x30, 0x94, 0x2C });
    DWORD firstLotAddress = itemLotsAddress + 0x20;

    int curOffset, curItemLotId, curItemId;
    int itemToReplaceId = 60375000;
    int pointer = firstLotAddress;

    while (true) {

        ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pointer), &curItemLotId, sizeof(int), NULL);
        ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pointer + 4), &curOffset, sizeof(int), NULL);
        
        // id of the last itemLot from ItemLotParam2_Other
        if (curItemLotId == 99996008) {
            break;
        }

        // skip if it's a bird trade
        // TODO: maybe make bird trades a location
        if (curItemLotId >= 50000000 && curItemLotId <= 50000303) {
            pointer += 12;
            continue;
        }

        for (int i = 0; i < 10; i++) {
            ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(itemLotsAddress + curOffset + i * 4), &curItemId, sizeof(int), NULL);
            if (curItemId == 0 || curItemId == 10) {
                continue;
            }
            WriteProcessMemory(GetCurrentProcess(), (LPVOID*)(itemLotsAddress + curOffset + i * 4), &itemToReplaceId, sizeof(int), NULL);
        }
        pointer += 12;
    }
}

std::list<int64_t> locations;
std::list<int64_t> Hooks::getLocations()
{
    return locations;
}

void Hooks::clearLocations(std::list<int64_t> locationsToRemove) {
    for (auto const& i : locationsToRemove) {
        locations.remove(i);
    }
}

// ============================= HOOKS =============================

INT __stdcall detourGetaddrinfo(PCSTR address, PCSTR port, const ADDRINFOA* pHints, PADDRINFOA* ppResult) {

    if (address && strcmp(address, "frpg2-steam-ope.fromsoftware.jp") == 0) {
        return EAI_FAIL;
    }

    return originalGetaddrinfo(address, port, pHints, ppResult);
}

int locationAmount = 0;
// this runs after AddToInventory
void __fastcall detourPickupItemLot(UINT_PTR thisPtr, void* Unknown, UINT_PTR idk) {
    int itemLotId = 0;
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(thisPtr + 8), &itemLotId, sizeof(itemLotId), 0);

    for (int i = 0; i < locationAmount; i++) {
        spdlog::debug("just picked up itemlot with id: ", itemLotId);
        locations.push_back(itemLotId);
        itemLotId++;
    }
    locationAmount = 0;

    return originalPickupItemLot(thisPtr, idk);
}

int locationId = -1;
// this runs before AddToInventory
void __fastcall detourGiveReward(UINT_PTR thisPtr, void* Unknown, UINT_PTR* pItemLot, INT idk1, INT idk2, INT idk3) {

    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pItemLot), &locationId, sizeof(locationId), NULL);
    spdlog::debug("was rewarded: ", locationId);
    return originalGiveReward(thisPtr, pItemLot, idk1, idk2, idk3);
}

char __fastcall detourAddToInventory(UINT_PTR thisPtr, void* Unknown, ItemStruct* itemsList, INT amountToGive, INT param_3) {

    for (int i = 0; i < 8; ++i) {
        Item* item = &itemsList->items[i];

        // if item was picked up from a location we don't give the player the item
        if (item->itemId != 60375000) break;
           
        showItem = false;
        
        // we are getting a reward
        if (locationId != -1) {
            locations.push_back(locationId+i);
        }
        // we picked up an item
        else {
            locationAmount++;
        }
    }

    if (!showItem) {
        locationId = -1;
        return 1;
    }

    return originalAddToInventory(thisPtr, itemsList, amountToGive, param_3);
}

void __fastcall detourShowItemPopup(UINT_PTR thisPtr, void* Unknown, UINT_PTR displayStruct) {
    if (!showItem) {
        showItem = true;
        return;
    }

    return originalShowItemPopup(thisPtr, displayStruct);
}

void __fastcall detourSelectMenuOption(UINT_PTR thisPtr, void* Unknown) {
    originalSelectMenuOption(thisPtr);

    int menuOptionId;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(thisPtr + 0xA4), &menuOptionId, sizeof(menuOptionId), NULL);

    if (menuOptionId == 1) {
        spdlog::debug("created a new game");
        GameHooks->overwriteItemLots();
    }
}

void __fastcall detourSelectSaveSlot(UINT_PTR thisPtr, void* Unknown) {
    int slotId;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(thisPtr + 0x74), &slotId, sizeof(slotId), NULL);

    spdlog::debug("loaded save: ", slotId);

    GameHooks->overwriteItemLots();

    originalSelectSaveSlot(thisPtr);
}

bool Hooks::initHooks() {

    HMODULE hModule = GetModuleHandleA("DarkSoulsII.exe");
    baseAddress = (DWORD)hModule;

    MH_Initialize();

    MH_CreateHookApi(L"ws2_32", "getaddrinfo", &detourGetaddrinfo, (LPVOID*)&originalGetaddrinfo);

    MH_CreateHook((LPVOID)(baseAddress + PickupItemLotFunction), &detourPickupItemLot, (LPVOID*)&originalPickupItemLot);
    MH_CreateHook((LPVOID)(baseAddress + GiveItemRewardsFunction), &detourGiveReward, (LPVOID*)&originalGiveReward);
    MH_CreateHook((LPVOID)(baseAddress + AddItemToInventoryFunction), &detourAddToInventory, (LPVOID*)&originalAddToInventory);
    originalCreatePopupStructure = reinterpret_cast<createPopupStructure_t>(baseAddress + PopUpStructFunction);
    MH_CreateHook((LPVOID)(baseAddress + ShowItemPopupFunction), &detourShowItemPopup, (LPVOID*)&originalShowItemPopup);
    MH_CreateHook((LPVOID)(baseAddress + SelectMenuOptionFunction), &detourSelectMenuOption, (LPVOID*)&originalSelectMenuOption);
    MH_CreateHook((LPVOID)(baseAddress + SelectSaveSlotFunction), &detourSelectSaveSlot, (LPVOID*)&originalSelectSaveSlot);

    MH_EnableHook(MH_ALL_HOOKS);

    return true;
}

int prevHp, curHp = -1;
bool Hooks::isPlayerDead() {
    prevHp = curHp;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)GetPointerAddress(baseAddress, 0x1150414, { 0x74, 0xFC }), &curHp, sizeof(int), NULL);
    if (prevHp != 0 && curHp == 0) {
        return true;
    }
    return false;
}

void Hooks::killPlayer() {
    int zeroHp = 0;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID*)GetPointerAddress(baseAddress, 0x1150414, { 0x74, 0xFC }), &zeroHp, sizeof(int), NULL);
}

bool Hooks::isPlayerInGame() {
    DWORD value;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)GetPointerAddress(baseAddress, 0x1150414, { 0x74 }), &value, sizeof(DWORD), NULL);
    if (value != 0) {
        return true;
    }
    return false;
}