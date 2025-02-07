#include <minhook.h>
#include <iostream>

#include "hooks.h"

bool showItem = true;
DWORD baseAddress;

getaddrinfo_t originalGetaddrinfo = nullptr;
checkLocation_t originalCheckLocation = nullptr;
itemGive_t originalItemGive = nullptr;
createPopupStructure_t originalCreatePopupStructure = nullptr;
showItemPopup_t originalShowItemPopup = nullptr;
selectMenuOption_t originalSelectMenuOption = nullptr;
selectSaveSlot_t originalSelectSaveSlot = nullptr;

// ============================= Utils =============================

DWORD GetPointerAddress(DWORD gameBaseAddr, DWORD address, std::vector<DWORD> offsets)
{
    DWORD offset_null = NULL;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(gameBaseAddr + address), &offset_null, sizeof(offset_null), 0);
    DWORD pointeraddress = offset_null; // the address we need
    for (int i = 0; i < offsets.size() - 1; i++) // we dont want to change the last offset value so we do -1
    {
        ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pointeraddress + offsets.at(i)), &pointeraddress, sizeof(pointeraddress), 0);
    }
    return pointeraddress += offsets.at(offsets.size() - 1); // adding the last offset
}

// TODO: receive the other item information like amount, upgrades and infusions
void giveItems(std::vector<int> ids) {

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

    originalItemGive(GetPointerAddress(baseAddress, 0x1150414, { 0x60, 0x8, 0x8, 0x0 }), &itemStruct, ids.size(), 0);
    originalCreatePopupStructure(displayStruct, &itemStruct, ids.size(), 1);
    originalShowItemPopup(GetPointerAddress(baseAddress, 0x1150414, { 0xCC4, 0x0 }), displayStruct);

    VirtualFree((LPVOID)displayStruct, 0, MEM_RELEASE);
}

void overwriteItemLots() {

    DWORD itemLotsAddress = GetPointerAddress(baseAddress, 0x1150414, { 0x60, 0x30, 0x94, 0x2C });
    DWORD firstLotAddress = itemLotsAddress + 0x20;

    int curOffset, curItemLotId, curItemId;
    int itemToReplaceId = 60375000;
    int pointer = firstLotAddress;

    while (true) {

        ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pointer), &curItemLotId, sizeof(int), NULL);
        ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pointer + 4), &curOffset, sizeof(int), NULL);

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

        if (curItemLotId == 99996008) {
            break;
        }
    }
}

// ============================= HOOKS =============================

INT __stdcall detourGetaddrinfo(PCSTR address, PCSTR port, const ADDRINFOA* pHints, PADDRINFOA* ppResult) {

    if (address && strcmp(address, "frpg2-steam-ope.fromsoftware.jp") == 0) {
        return EAI_FAIL;
    }

    return originalGetaddrinfo(address, port, pHints, ppResult);
}

void __fastcall detourCheckLocation(UINT_PTR thisPtr, void* Unknown, UINT_PTR idk) {
    int itemLotId = 0;
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(thisPtr + 8), &itemLotId, sizeof(itemLotId), 0);

    std::cout << "just checked item location: " << itemLotId << std::endl;

    return originalCheckLocation(thisPtr, idk);
}

char __fastcall detourItemGive(UINT_PTR thisPtr, void* Unknown, ItemStruct* itemsList, INT amountToGive, INT param_3) {

    for (int i = 0; i < 8; ++i) {
        Item* item = &itemsList->items[i];

        // if item was picked up from a location
        // we don't give the player the item
        if (item->itemId == 60375000) {
            showItem = false;
            return 1;
        }
    }

    return originalItemGive(thisPtr, itemsList, amountToGive, param_3);
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
        std::cout << "created a new game" << std::endl;
        overwriteItemLots();
    }
}

void __fastcall detourSelectSaveSlot(UINT_PTR thisPtr, void* Unknown) {

    int slotId;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(thisPtr + 0x74), &slotId, sizeof(slotId), NULL);

    std::cout << "loaded save " << slotId << std::endl;

    overwriteItemLots();

    originalSelectSaveSlot(thisPtr);
}

bool initHooks() {

    HMODULE hModule = GetModuleHandleA("DarkSoulsII.exe");
    baseAddress = (DWORD)hModule;

    MH_Initialize();

    MH_CreateHookApi(L"ws2_32", "getaddrinfo", &detourGetaddrinfo, (LPVOID*)&originalGetaddrinfo);

    MH_CreateHook((LPVOID)(baseAddress + 0x257060), &detourCheckLocation, (LPVOID*)&originalCheckLocation);
    MH_CreateHook((LPVOID)(baseAddress + 0x22AD20), &detourItemGive, (LPVOID*)&originalItemGive);
    originalCreatePopupStructure = reinterpret_cast<createPopupStructure_t>(baseAddress + 0x11F430);
    MH_CreateHook((LPVOID)(baseAddress + 0x4FA9B0), &detourShowItemPopup, (LPVOID*)&originalShowItemPopup);
    MH_CreateHook((LPVOID)(baseAddress + 0x18C160), &detourSelectMenuOption, (LPVOID*)&originalSelectMenuOption);
    MH_CreateHook((LPVOID)(baseAddress + 0x189450), &detourSelectSaveSlot, (LPVOID*)&originalSelectSaveSlot);

    MH_EnableHook(MH_ALL_HOOKS);

    return true;
}