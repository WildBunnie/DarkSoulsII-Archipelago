#include "offsets.h"

#include <vector>
#include <cstdint>

namespace function_offsets {
#ifdef _M_IX86
    uintptr_t give_items_on_reward = 0x21D3C0;
    uintptr_t give_items_on_pickup = 0x25BD90;
    uintptr_t give_shop_item = 0x22B340;
    uintptr_t items_fit_in_inventory = 0x22BB60;
    //uintptr_t add_shop_item_to_inventory; // this is the same as AddItemsToInventory in vanilla
    uintptr_t add_items_to_inventory = 0x22AD20;
    uintptr_t create_popup_structure = 0x11F430;
    uintptr_t show_item_popup = 0x4FA9B0;
    uintptr_t get_item_info = 0x4ff6e0;
    uintptr_t get_hovering_item_info = 0x105A40;
    uintptr_t remove_item_from_inventory = 0x233ED0;
#elif defined(_M_X64)
    uintptr_t give_items_on_reward = 0x199CC0;
    uintptr_t give_items_on_pickup = 0x1E4590;
    uintptr_t give_shop_item = 0x1A76A0;
    uintptr_t items_fit_in_inventory = 0x1ABA90;
    uintptr_t add_shop_item_to_inventory = 0x1AA810;
    uintptr_t add_items_to_inventory = 0x1A7470;
    uintptr_t create_popup_structure = 0x5D950;
    uintptr_t show_item_popup = 0x501080;
    uintptr_t get_item_info = 0x503620;
    uintptr_t get_hovering_item_info = 0x38170;
    uintptr_t remove_item_from_inventory = 0x1AF1E0;
#endif
};

namespace PointerOffsets {
#ifdef _M_IX86
    uintptr_t BaseA = 0x1150414;
    std::vector<uintptr_t> GameState = { 0xDEC };
    std::vector<uintptr_t> HP = { 0x74, 0xFC };
    std::vector<uintptr_t> AvailableItemBag = { 0x60, 0x8, 0x8, 0x0 };
    std::vector<uintptr_t> ItemGiveWindow = { 0xCC4, 0x0 };
    std::vector<uintptr_t> WorldFlags = { 0x44, 0x10, 0x28, 0x0, 0x0 };
#elif defined(_M_X64)
    uintptr_t BaseA = 0x16148F0;
    std::vector<uintptr_t> GameState = { 0x24AC };
    std::vector<uintptr_t> HP = { 0xD0, 0x168 };
    std::vector<uintptr_t> AvailableItemBag = { 0xA8, 0x10, 0x10, 0x0 };
    std::vector<uintptr_t> ItemGiveWindow = { 0x22E0, 0x0 };
    std::vector<uintptr_t> WorldFlags = { 0x70, 0x20, 0x18, 0x0 };
#endif
};

namespace Patches {
#ifdef _M_IX86
    std::vector<uint8_t> noWeaponReqPatch = { 0xC2, 0x04, 0x00 };
    std::vector<uint8_t> menuWeaponReqPatch = {
        0xC7, 0x46, 0x70, 0x01, 0x00, 0x00, 0x00, 0x90,
        0xC7, 0x46, 0x72, 0x01, 0x00, 0x00, 0x00, 0x90,
        0xC7, 0x46, 0x74, 0x01, 0x00, 0x00, 0x00, 0x90,
        0xC7, 0x46, 0x76, 0x01, 0x00, 0x00, 0x00, 0x90
    };
    std::vector<uint8_t> noPowerStanceReq = { 0x66, 0x0F, 0xEF, 0xC9 };
    std::vector<uint8_t> noSpellCastReq = { 0xEB };
    std::vector<uint8_t> noSpellMenuIntReq = { 0x66, 0xBE, 0x00, 0x00 };
    std::vector<uint8_t> noSpellMenuFthReq = { 0x66, 0xBA, 0x00, 0x00 };
    std::vector<uint8_t> infiniteTorch = { 0x90, 0x90, 0x90, 0x90 };
    std::vector<uint8_t> UnbreakableChests = { 0x80, 0x78, 0x14, 0x78, 0x74, 0xE6 };
    std::vector<uint8_t> DissapearingChestItems = { 0x90, 0x90 };
#elif defined(_M_X64)
    std::vector<uint8_t> noWeaponReqPatch = { 0xC3, 0x90, 0x90, 0x90, 0x90 };
    std::vector<uint8_t> menuWeaponReqPatch = {
        0x90, 0x90, 0x90, 0x90, 0xB0, 0x01, 0x66, 0x89, 0x4B, 0x70,
        0x90, 0x90, 0x90, 0x90, 0x66, 0x89, 0x4B, 0x72, 0x90, 0x90,
        0x90, 0x90, 0x66, 0x89, 0x4B, 0x74, 0x90, 0x90, 0x90, 0x90
    };
    std::vector<uint8_t> noPowerStanceReq = { 0x66, 0x0F, 0xEF, 0xC0, 0x90, 0x90, 0x90, 0x90 };
    std::vector<uint8_t> noSpellCastReq = { 0xEB };
    std::vector<uint8_t> noSpellMenuIntReq = { 0x31, 0xC9, 0x90, 0x90 };
    std::vector<uint8_t> noSpellMenuFthReq = { 0x31, 0xC9, 0x90, 0x90 };
    std::vector<uint8_t> infiniteTorch = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    std::vector<uint8_t> UnbreakableChests = { 0x80, 0x78, 0x14, 0x78, 0x74, 0xE3 };
    std::vector<uint8_t> DissapearingChestItems = { 0x90, 0x90 };
#endif
};

namespace PatchesOffsets {
#ifdef _M_IX86
    uintptr_t noWeaponReqPatchOffset = 0x372ff0;
    uintptr_t menuWeaponReqPatchOffset = 0x14E50A2;
    uintptr_t noPowerStanceStrReq = 0x378398;
    uintptr_t noPowerStanceDexReq = 0x3783D7;
    uintptr_t noSpellCastIntReq = 0x3C02CF;
    uintptr_t noSpellCastFthReq = 0x3C02E7;
    uintptr_t noSpellMenuIntReq = 0x236E72;
    uintptr_t noSpellMenuFthReq = 0x236E7A;
    uintptr_t infiniteTorchOffset = 0x23950E;
    uintptr_t UnbreakableChests = 0x24EDFC;
    uintptr_t DissapearingChestItems = 0x259FDA;
#elif defined(_M_X64)
    uintptr_t noWeaponReqPatchOffset = 0x34A980;
    uintptr_t menuWeaponReqPatchOffset = 0x201EC0;
    uintptr_t noPowerStanceStrReq = 0x35025C;
    uintptr_t noPowerStanceDexReq = 0x35028B;
    uintptr_t noSpellCastIntReq = 0x397915;
    uintptr_t noSpellCastFthReq = 0x39792C;
    uintptr_t noSpellMenuIntReq = 0x1B8E7C;
    uintptr_t noSpellMenuFthReq = 0x1B8E87;
    uintptr_t infiniteTorchOffset = 0x1BBD1C;
    uintptr_t UnbreakableChests = 0x1D3C29;
    uintptr_t DissapearingChestItems = 0x1E35E3;
#endif
};

namespace ParamOffsets {
#ifdef _M_IX86
    std::vector<uintptr_t> ShopLineupParam = { 0x60, 0x58, 0x94, 0x0 };
    std::vector<uintptr_t> ItemParam = { 0x60, 0x10, 0x94, 0x0 };
    std::vector<uintptr_t> ItemLotParam2_Other = { 0x60, 0x30, 0x94, 0x0 };
    std::vector<uintptr_t> ItemLotParam2_Chr = { 0x60, 0x28, 0x94, 0x0 };
#elif defined(_M_X64)
    std::vector<uintptr_t> ShopLineupParam = { 0xA8, 0xB0, 0xD8, 0x0 };
    std::vector<uintptr_t> ItemParam = { 0xA8, 0x20, 0xD8, 0x0 };
    std::vector<uintptr_t> ItemLotParam2_Other = { 0xA8, 0x60, 0xD8, 0x0 };
    std::vector<uintptr_t> ItemLotParam2_Chr = { 0xA8, 0x50, 0xD8, 0x0 };
#endif
}

std::vector<int> unused_item_ids = {
   3400000, 3401000, 21001100, 21001101, 21001102, 21001103, 21600000,
   21610000, 21620000, 21630000, 21640000, 21650000, 21660000, 21670000,
   21680000, 21690000, 21700000, 21710000, 26590000, 26750000, 26770000,
   26800000, 26900000, 27521000, 60375000, 65240000, 65250000, 65260000,
   65270000, 65280000, 65290000, 900008182, 900008183
};