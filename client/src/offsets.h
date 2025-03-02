#include <vector>

#pragma once

namespace FunctionOffsets {
#ifdef _M_IX86
    uintptr_t GiveItemsOnReward = 0x21D3C0;
    uintptr_t GiveItemsOnPickup = 0x25bff0;
    uintptr_t AddItemsToInventory = 0x22AD20;
    uintptr_t CreatePopUpStruct = 0x11F430;
    uintptr_t ShowItemPopup = 0x4FA9B0;
#elif defined(_M_X64)
    uintptr_t GiveItemsOnReward = 0x199CC0;
    uintptr_t GiveItemsOnPickup = 0x1E4590; // this is not the same function as in vanilla
    uintptr_t AddItemsToInventory = 0x1A7470;
    uintptr_t CreatePopUpStruct = 0x5D950;
    uintptr_t ShowItemPopup = 0x501080;
#endif
};

// names/values are mostly taken from META
// https://github.com/pseudostripy/DS2S-META/blob/master/DS2S%20META/Utils/Offsets/OffsetClasses/DS2REData.cs
namespace PointerOffsets {
#ifdef _M_IX86
    uintptr_t BaseA = 0x1150414;
    std::vector<uintptr_t> PlayerCtrl = { 0x74 };
    std::vector<uintptr_t> HP = { 0x74, 0xFC }; 
    std::vector<uintptr_t> AvailableItemBag = { 0x60, 0x8, 0x8, 0x0 };
    std::vector<uintptr_t> ItemGiveWindow = { 0xCC4, 0x0 };
#elif defined(_M_X64)
    uintptr_t BaseA = 0x16148F0;
    std::vector<uintptr_t> PlayerCtrl = { 0xD0 };
    std::vector<uintptr_t> HP = { 0xD0, 0x168 };
    std::vector<uintptr_t> AvailableItemBag = { 0xA8, 0x10, 0x10, 0x0 };
    std::vector<uintptr_t> ItemGiveWindow = { 0x22E0, 0x0 };
#endif
};