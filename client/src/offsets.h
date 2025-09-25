#pragma once

#include <vector>
#include <cstdint>

namespace function_offsets {
#ifdef _M_IX86
    const uintptr_t give_items_on_reward = 0x21D3C0;
    const uintptr_t give_items_on_pickup = 0x25BD90;
    const uintptr_t give_shop_item = 0x22B340;
    const uintptr_t items_fit_in_inventory = 0x22BB60;
    const uintptr_t add_item_to_inventory = 0x229C00;
    const uintptr_t add_items_to_inventory = 0x22AD20;
    const uintptr_t create_popup_structure = 0x11F430;
    const uintptr_t show_item_popup = 0x4FA9B0;
    const uintptr_t get_item_info = 0x4ff6e0;
    const uintptr_t get_hovering_item_info = 0x105A40;
    const uintptr_t remove_item_from_inventory = 0x233ED0;
    const uintptr_t virtual_to_archive_path = 0x57EB70;
    const uintptr_t equip_item = 0x233530;
    const uintptr_t unequip_item = 0x232B80;
#elif defined(_M_X64)
    const uintptr_t give_items_on_reward = 0x199CC0;
    const uintptr_t give_items_on_pickup = 0x1E4590;
    const uintptr_t give_shop_item = 0x1A76A0;
    const uintptr_t items_fit_in_inventory = 0x1ABA90;
    const uintptr_t add_item_to_inventory = 0x1AA810;
    const uintptr_t add_items_to_inventory = 0x1A7470;
    const uintptr_t create_popup_structure = 0x5D950;
    const uintptr_t show_item_popup = 0x501080;
    const uintptr_t get_item_info = 0x503620;
    const uintptr_t get_hovering_item_info = 0x38170;
    const uintptr_t remove_item_from_inventory = 0x1AF1E0;
    const uintptr_t virtual_to_archive_path = 0x89C5A0;
    const uintptr_t equip_item = 0x1B3D50;
    const uintptr_t unequip_item = 0x1B4330;
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
    const std::vector<uintptr_t> equiped_rings = { 0x74, 0xF0, 0xA04 };
#elif defined(_M_X64)
    const uintptr_t base_a = 0x16148F0;
    const std::vector<uintptr_t> game_state = { 0x24AC };
    const std::vector<uintptr_t> hp = { 0xD0, 0x168 };
    const std::vector<uintptr_t> available_item_bag = { 0xA8, 0x10, 0x10, 0x0 };
    const std::vector<uintptr_t> item_give_window = { 0x22E0, 0x0 };
    const std::vector<uintptr_t> world_flags = { 0x70, 0x20, 0x18, 0x0 };
    const std::vector<uintptr_t> equiped_rings = { 0xD0, 0x378, 0x20, 0x458 };
#endif
};

namespace patches {
#ifdef _M_IX86
    const std::vector<uint8_t> infinite_torch = { 0x90, 0x90, 0x90, 0x90 };
    const std::vector<uint8_t> unbreakable_chests = { 0x80, 0x78, 0x14, 0x78, 0x74, 0xE6 };
#elif defined(_M_X64)
    const std::vector<uint8_t> infinite_torch = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    const std::vector<uint8_t> unbreakable_chests = { 0x80, 0x78, 0x14, 0x78, 0x74, 0xE3 };
#endif
};

namespace patches_offsets {
#ifdef _M_IX86
    const uintptr_t infinite_torch = 0x23950E;
    const uintptr_t unbreakable_chests = 0x24EDFC;
#elif defined(_M_X64)
    const uintptr_t infinite_torch = 0x1BBD1C;
    const uintptr_t unbreakable_chests = 0x1D3C29;
#endif
};

namespace param_offsets {
#ifdef _M_IX86
    const std::vector<uintptr_t> shop_lineup_param = { 0x60, 0x58, 0x94, 0x0 };
    const std::vector<uintptr_t> item_param = { 0x60, 0x10, 0x94, 0x0 };
    const std::vector<uintptr_t> item_lot_param2_other = { 0x60, 0x30, 0x94, 0x0 };
    const std::vector<uintptr_t> item_lot_param2_chr = { 0x60, 0x28, 0x94, 0x0 };
    const std::vector<uintptr_t> player_status_param = { 0x18, 0x398, 0x94, 0x0 };
    const std::vector<uintptr_t> weapon_param = { 0x18, 0x210, 0x94, 0x0 };
    const std::vector<uintptr_t> spell_param = { 0x18, 0x2A0, 0x94, 0x0 };
    const std::vector<uintptr_t> armor_param = { 0x18, 0x250, 0x94, 0x0 };
    const std::vector<uintptr_t> ring_param = { 0x18, 0x298, 0x94, 0x0 };
#elif defined(_M_X64)
    const std::vector<uintptr_t> shop_lineup_param = { 0xA8, 0xB0, 0xD8, 0x0 };
    const std::vector<uintptr_t> item_param = { 0xA8, 0x20, 0xD8, 0x0 };
    const std::vector<uintptr_t> item_lot_param2_other = { 0xA8, 0x60, 0xD8, 0x0 };
    const std::vector<uintptr_t> item_lot_param2_chr = { 0xA8, 0x50, 0xD8, 0x0 };
    const std::vector<uintptr_t> player_status_param = { 0x18, 0x730, 0xD8, 0x0 };
    const std::vector<uintptr_t> weapon_param = { 0x18, 0x420, 0xD8, 0x0 };
    const std::vector<uintptr_t> spell_param = { 0x18, 0x540, 0xD8, 0x0 };
    const std::vector<uintptr_t> armor_param = { 0x18, 0x4A0, 0xD8, 0x0 };
    const std::vector<uintptr_t> ring_param = { 0x18, 0x530, 0xD8, 0x0 };
#endif
}
