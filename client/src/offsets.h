#pragma once

#include <vector>
#include <cstdint>

namespace function_offsets {
    extern uintptr_t give_items_on_reward;
    extern uintptr_t give_items_on_pickup;
    extern uintptr_t give_shop_item;
    extern uintptr_t items_fit_in_inventory;
    extern uintptr_t add_shop_item_to_inventory;
    extern uintptr_t add_items_to_inventory;
    extern uintptr_t create_popup_structure;
    extern uintptr_t show_item_popup;
    extern uintptr_t get_item_info;
    extern uintptr_t get_hovering_item_info;
    extern uintptr_t remove_item_from_inventory;
};

namespace PointerOffsets {
    extern uintptr_t BaseA;
    extern std::vector<uintptr_t> GameState;
    extern std::vector<uintptr_t> HP;
    extern std::vector<uintptr_t> AvailableItemBag;
    extern std::vector<uintptr_t> ItemGiveWindow;
    extern std::vector<uintptr_t> WorldFlags;
};

namespace Patches {
    extern std::vector<uint8_t> noWeaponReqPatch;
    extern std::vector<uint8_t> menuWeaponReqPatch;
    extern std::vector<uint8_t> noPowerStanceReq;
    extern std::vector<uint8_t> noSpellCastReq;
    extern std::vector<uint8_t> noSpellMenuIntReq;
    extern std::vector<uint8_t> noSpellMenuFthReq;
    extern std::vector<uint8_t> infiniteTorch;
    extern std::vector<uint8_t> UnbreakableChests;
    extern std::vector<uint8_t> DissapearingChestItems;
};

namespace PatchesOffsets {
    extern uintptr_t noWeaponReqPatchOffset;
    extern uintptr_t menuWeaponReqPatchOffset;
    extern uintptr_t noPowerStanceStrReq;
    extern uintptr_t noPowerStanceDexReq;
    extern uintptr_t noSpellCastIntReq;
    extern uintptr_t noSpellCastFthReq;
    extern uintptr_t noSpellMenuIntReq;
    extern uintptr_t noSpellMenuFthReq;
    extern uintptr_t infiniteTorchOffset;
    extern uintptr_t UnbreakableChests;
    extern uintptr_t DissapearingChestItems;
};

namespace ParamOffsets {
    extern std::vector<uintptr_t> ShopLineupParam;
    extern std::vector<uintptr_t> ItemParam;
    extern std::vector<uintptr_t> ItemLotParam2_Other;
    extern std::vector<uintptr_t> ItemLotParam2_Chr;
}

extern std::vector<int> unused_item_ids;