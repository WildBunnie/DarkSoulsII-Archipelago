#pragma once

#include <vector>
#include <cstdint>

namespace function_offsets {
#ifdef _M_IX86
    const uintptr_t give_items_on_reward = 0x21D3C0;
    const uintptr_t give_items_on_pickup = 0x25BD90;
    const uintptr_t give_shop_item = 0x22B340;
    const uintptr_t items_fit_in_inventory = 0x22BB60;
    // const uintptr_t add_shop_item_to_inventory = 0x22B340; // Same as AddItemsToInventory in vanilla
    const uintptr_t add_items_to_inventory = 0x22AD20;
    const uintptr_t create_popup_structure = 0x11F430;
    const uintptr_t show_item_popup = 0x4FA9B0;
    const uintptr_t get_item_info = 0x4ff6e0;
    const uintptr_t get_hovering_item_info = 0x105A40;
    const uintptr_t remove_item_from_inventory = 0x233ED0;
    const uintptr_t virtual_to_archive_path = 0x57EB70;
#elif defined(_M_X64)
    const uintptr_t give_items_on_reward = 0x199CC0;
    const uintptr_t give_items_on_pickup = 0x1E4590;
    const uintptr_t give_shop_item = 0x1A76A0;
    const uintptr_t items_fit_in_inventory = 0x1ABA90;
    const uintptr_t add_shop_item_to_inventory = 0x1AA810;
    const uintptr_t add_items_to_inventory = 0x1A7470;
    const uintptr_t create_popup_structure = 0x5D950;
    const uintptr_t show_item_popup = 0x501080;
    const uintptr_t get_item_info = 0x503620;
    const uintptr_t get_hovering_item_info = 0x38170;
    const uintptr_t remove_item_from_inventory = 0x1AF1E0;
    const uintptr_t virtual_to_archive_path = 0x89C5A0;
#endif
};

namespace pointer_offsets {
#ifdef _M_IX86
    const uintptr_t base_a = 0x1150414;
    const std::vector<uintptr_t> game_state = { 0xDEC };
    const std::vector<uintptr_t> hp = { 0x74, 0xFC };
    const std::vector<uintptr_t> available_item_bag = { 0x60, 0x8, 0x8, 0x0 };
    const std::vector<uintptr_t> item_give_window = { 0xCC4, 0x0 };
    const std::vector<uintptr_t> world_flags = { 0x44, 0x10, 0x28, 0x0, 0x0 };
#elif defined(_M_X64)
    const uintptr_t base_a = 0x16148F0;
    const std::vector<uintptr_t> game_state = { 0x24AC };
    const std::vector<uintptr_t> hp = { 0xD0, 0x168 };
    const std::vector<uintptr_t> available_item_bag = { 0xA8, 0x10, 0x10, 0x0 };
    const std::vector<uintptr_t> item_give_window = { 0x22E0, 0x0 };
    const std::vector<uintptr_t> world_flags = { 0x70, 0x20, 0x18, 0x0 };
#endif
};

namespace patches {
#ifdef _M_IX86
    const std::vector<uint8_t> no_weapon_req_patch = { 0xC2, 0x04, 0x00 };
    const std::vector<uint8_t> menu_weapon_req_patch = {
        0xC7, 0x46, 0x70, 0x01, 0x00, 0x00, 0x00, 0x90,
        0xC7, 0x46, 0x72, 0x01, 0x00, 0x00, 0x00, 0x90,
        0xC7, 0x46, 0x74, 0x01, 0x00, 0x00, 0x00, 0x90,
        0xC7, 0x46, 0x76, 0x01, 0x00, 0x00, 0x00, 0x90
    };
    const std::vector<uint8_t> no_power_stance_req = { 0x66, 0x0F, 0xEF, 0xC9 };
    const std::vector<uint8_t> no_spell_cast_req = { 0xEB };
    const std::vector<uint8_t> no_spell_menu_int_req = { 0x66, 0xBE, 0x00, 0x00 };
    const std::vector<uint8_t> no_spell_menu_fth_req = { 0x66, 0xBA, 0x00, 0x00 };
    const std::vector<uint8_t> infinite_torch = { 0x90, 0x90, 0x90, 0x90 };
    const std::vector<uint8_t> unbreakable_chests = { 0x80, 0x78, 0x14, 0x78, 0x74, 0xE6 };
    const std::vector<uint8_t> disappearing_chest_items = { 0x90, 0x90 };
    const std::vector<uint8_t> equip_load = { 0xD9, 0xEE, 0x90, 0x90, 0x90 };
    const std::vector<uint8_t> equip_load_menu = { 0x31, 0xD2, 0x90 };
#elif defined(_M_X64)
    const std::vector<uint8_t> no_weapon_req_patch = { 0xC3, 0x90, 0x90, 0x90, 0x90 };
    const std::vector<uint8_t> menu_weapon_req_patch = {
        0x90, 0x90, 0x90, 0x90, 0xB0, 0x01, 0x66, 0x89, 0x4B, 0x70,
        0x90, 0x90, 0x90, 0x90, 0x66, 0x89, 0x4B, 0x72, 0x90, 0x90,
        0x90, 0x90, 0x66, 0x89, 0x4B, 0x74, 0x90, 0x90, 0x90, 0x90
    };
    const std::vector<uint8_t> no_power_stance_req = { 0x66, 0x0F, 0xEF, 0xC0, 0x90, 0x90, 0x90, 0x90 };
    const std::vector<uint8_t> no_spell_cast_req = { 0xEB };
    const std::vector<uint8_t> no_spell_menu_int_req = { 0x31, 0xC9, 0x90, 0x90 };
    const std::vector<uint8_t> no_spell_menu_fth_req = { 0x31, 0xC9, 0x90, 0x90 };
    const std::vector<uint8_t> infinite_torch = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    const std::vector<uint8_t> unbreakable_chests = { 0x80, 0x78, 0x14, 0x78, 0x74, 0xE3 };
    const std::vector<uint8_t> disappearing_chest_items = { 0x90, 0x90 };
    const std::vector<uint8_t> equip_load = { 0x31, 0xC9, 0x89, 0x4B, 0x44 };
    const std::vector<uint8_t> equip_load_menu = { 0x89, 0x81, 0x9C, 0xA2, 0x00, 0x00, 0x90 };
#endif
};

namespace patches_offsets {
#ifdef _M_IX86
    const uintptr_t no_weapon_req = 0x372ff0;
    const uintptr_t menu_weapon_req = 0x14E50A2;
    const uintptr_t no_power_stance_str_req = 0x378398;
    const uintptr_t no_power_stance_dex_req = 0x3783D7;
    const uintptr_t no_spell_cast_int_req = 0x3C02CF;
    const uintptr_t no_spell_cast_fth_req = 0x3C02E7;
    const uintptr_t no_spell_menu_int_req = 0x236E72;
    const uintptr_t no_spell_menu_fth_req = 0x236E7A;
    const uintptr_t infinite_torch = 0x23950E;
    const uintptr_t unbreakable_chests = 0x24EDFC;
    const uintptr_t disappearing_chest_items = 0x259FDA;
    const uintptr_t equip_load = 0x376B87;
    const uintptr_t equip_load_menu = 0x14C83D;
#elif defined(_M_X64)
    const uintptr_t no_weapon_req = 0x34A980;
    const uintptr_t menu_weapon_req = 0x201EC0;
    const uintptr_t no_power_stance_str_req = 0x35025C;
    const uintptr_t no_power_stance_dex_req = 0x35028B;
    const uintptr_t no_spell_cast_int_req = 0x397915;
    const uintptr_t no_spell_cast_fth_req = 0x39792C;
    const uintptr_t no_spell_menu_int_req = 0x1B8E7C;
    const uintptr_t no_spell_menu_fth_req = 0x1B8E87;
    const uintptr_t infinite_torch = 0x1BBD1C;
    const uintptr_t unbreakable_chests = 0x1D3C29;
    const uintptr_t disappearing_chest_items = 0x1E35E3;
    const uintptr_t equip_load = 0x34A79E;
    const uintptr_t equip_load_menu = 0x99D71;
#endif
};

namespace param_offsets {
#ifdef _M_IX86
    const std::vector<uintptr_t> shop_lineup_param = { 0x60, 0x58, 0x94, 0x0 };
    const std::vector<uintptr_t> item_param = { 0x60, 0x10, 0x94, 0x0 };
    const std::vector<uintptr_t> item_lot_param2_other = { 0x60, 0x30, 0x94, 0x0 };
    const std::vector<uintptr_t> item_lot_param2_chr = { 0x60, 0x28, 0x94, 0x0 };
#elif defined(_M_X64)
    const std::vector<uintptr_t> shop_lineup_param = { 0xA8, 0xB0, 0xD8, 0x0 };
    const std::vector<uintptr_t> item_param = { 0xA8, 0x20, 0xD8, 0x0 };
    const std::vector<uintptr_t> item_lot_param2_other = { 0xA8, 0x60, 0xD8, 0x0 };
    const std::vector<uintptr_t> item_lot_param2_chr = { 0xA8, 0x50, 0xD8, 0x0 };
#endif
}
