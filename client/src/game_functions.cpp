#include "game_functions.h"

#include "offsets.h"
#include "memory.h"
#include "ds2.h"
#include "params.h"

#include "spdlog/spdlog.h"

#include <iostream>
#include <map>
#include <set>

struct WorldFlagOffset {
    uint32_t offset;
    uint8_t bit_start;
};

std::map<int, WorldFlagOffset> statue_offsets = {
#ifdef _M_X64
    {1, {0x9D2, 5}},  // Unpetrify Statue in Things Betwixt
    {2, {0x152, 7}},  // Unpetrify Rosabeth of Melfia
    {3, {0xCED, 5}},  // Unpetrify Statue in Heide's Tower of Flame
    {4, {0xA33, 0}},  // Unpetrify Statue in Lost Bastille
    {5, {0x15E, 2}},  // Unpetrify Straid of Olaphis
    {6, {0xAC9, 4}},  // Unpetrify Statue in Black Gulch
    {7, {0xB13, 7}},  // Unpetrify Statue near Manscorpion Tark
    {8, {0xB12, 5}},  // Unpetrify Statue near Black Knight Halberd
    {9, {0xB13, 5}},  // Unpetrify Statue Blocking the Chest in Shaded Ruins
    {10, {0xB12, 3}}, // Unpetrify Lion Mage Set Statue in Shaded Ruins
    {11, {0xB12, 1}}, // Unpetrify Fang Key Statue in Shaded Ruins
    {12, {0xB13, 6}}, // Unpetrify Warlock Mask Statue in Shaded Ruins
    {13, {0xB78, 1}}, // Unpetrify Milfanito Entrance Statue
    {14, {0xCA7, 5}}, // Unpetrify Cyclops Statue in Aldia's Keep
    {15, {0xCA7, 4}}, // Unpetrify Left Cage Statue in Aldia's Keep
    {16, {0xCA7, 3}}, // Unpetrify Right Cage Statue in Aldia's Keep
    {17, {0xCBD, 1}}  // Unpetrify Statue in Dragon Aerie
#elif defined(_M_IX86)
    { 2, {0x14a, 7} },  // Unpetrify Rosabeth of Melfia
    {5, {0x156, 2}},  // Unpetrify Straid of Olaphis
    {8, {0xB0A, 5}},  // Unpetrify Statue near Black Knight Halberd
    {10, {0xB0A, 3}}, // Unpetrify Lion Mage Set Statue in Shaded Ruins
    {11, {0xB0A, 1}}, // Unpetrify Fang Key Statue in Shaded Ruins
    {14, {0xC9F, 5}}  // Unpetrify Cyclops Statue in Aldia's Keep
#endif
};

std::map<int32_t, int8_t> item_categories;
std::map<ItemCategory, int32_t> equip_slots = {
    { Weapon,      0 },
    { Shield,      1 },
    { HeadArmor,   6 },
    { ChestArmor,  7 },
    { HandsArmor,  8 },
    { LegsArmor,   9 },
    { Ring,       10 }
};

int ring_index = 0;

void equip_last_received_item()
{
    uintptr_t base_address = get_base_address();

    equip_item_t equip_item = (equip_item_t)(base_address + function_offsets::equip_item);
    unequip_item_t unequip_item = (unequip_item_t)(base_address + function_offsets::unequip_item);

    uintptr_t inventory_ptr = resolve_pointer(base_address, pointer_offsets::base_a, pointer_offsets::available_item_bag);

#ifdef _M_IX86
    uintptr_t ptr = read_value<uintptr_t>(inventory_ptr + 0x8);
    uintptr_t last_inventory_slot_ptr = read_value<uintptr_t>(ptr + 0x1E0C0);
    int32_t item_id = read_value<int32_t>(last_inventory_slot_ptr + 0x10);
    int16_t inventory_slot_id = read_value<int16_t>(last_inventory_slot_ptr + 0x14);
#elif defined(_M_X64)
    uintptr_t ptr = read_value<uintptr_t>(inventory_ptr + 0x10);
    uintptr_t last_inventory_slot_ptr = read_value<uintptr_t>(ptr + 0x25980);
    int32_t item_id = read_value<int32_t>(last_inventory_slot_ptr + 0x14);
    int16_t inventory_slot_id = read_value<int16_t>(last_inventory_slot_ptr + 0x1C);
#endif

    if (item_categories.empty()) {
        item_categories = get_item_categories();
    }

    ItemCategory category = static_cast<ItemCategory>(item_categories[item_id]);

    if (category == Ring) {
        uintptr_t equipped_rings_ptr = resolve_pointer(base_address, pointer_offsets::base_a, pointer_offsets::equiped_rings);

        // unequip ring if its already equiped
        for (int i = 0; i < 4; i++) {
            int32_t equipped_ring = read_value<int32_t>(equipped_rings_ptr + (i * 4));

            // if we set the last digit to 0 we get the +0 ring
            int32_t base_equipped_ring = (equipped_ring / 10) * 10;
            int32_t base_ring_to_equip = (item_id / 10) * 10;

            if (base_equipped_ring == base_ring_to_equip) {
                unequip_item(ptr, equip_slots[category] + i);
                break;
            }
        }

        equip_item(ptr, equip_slots[category] + ring_index, item_id, inventory_slot_id);

        ring_index++;
        if (ring_index > 3) {
            ring_index = 0;
        }
    }
    else if (category == Weapon) {
        if (melee_weapons.contains(item_id)) {
            equip_item(ptr, equip_slots[category], item_id, inventory_slot_id);
        }
        else {
            equip_item(ptr, 3, item_id, inventory_slot_id); // non melee weapons go to left weapon 2
        }
    }
    else if (equip_slots.contains(category)) {
        equip_item(ptr, equip_slots[category], item_id, inventory_slot_id);
    }
}

void give_items(ItemStruct items, int num_items)
{
    uintptr_t base_address = get_base_address();
    unsigned char display_struct[0x200];

    add_items_to_inventory_t add_items = (add_items_to_inventory_t)(base_address + function_offsets::add_items_to_inventory);
    create_popup_structure_t create_popup = (create_popup_structure_t)(base_address + function_offsets::create_popup_structure);
    show_item_popup_t show_popup = (show_item_popup_t)(base_address + function_offsets::show_item_popup);

    add_items(resolve_pointer(base_address, pointer_offsets::base_a, pointer_offsets::available_item_bag), &items, num_items, 0);
    create_popup((uintptr_t)display_struct, &items, num_items, 1);
    show_popup(resolve_pointer(base_address, pointer_offsets::base_a, pointer_offsets::item_give_window), (uintptr_t)display_struct);
}

bool is_statue(int item_id)
{
    return statue_offsets.contains(item_id);
}

void unpetrify_statue(int statue_id)
{
    if (!statue_offsets.contains(statue_id)) {
        return;
    }

    WorldFlagOffset statueOffset = statue_offsets[statue_id];
    uintptr_t worldFlagsPtr = resolve_pointer(get_base_address(), pointer_offsets::base_a, pointer_offsets::world_flags);

    uint8_t value = read_value<uint8_t>(worldFlagsPtr + statueOffset.offset);
    value |= (1 << statueOffset.bit_start);
    write_value<uint8_t>(worldFlagsPtr + statueOffset.offset, value);
}

int32_t previous_hp, current_hp = -1;
bool player_just_died()
{
    previous_hp = current_hp;
    uintptr_t hp_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, pointer_offsets::hp);
    current_hp = read_value<int32_t>(hp_ptr);
    if (previous_hp != current_hp && previous_hp > 0 && current_hp <= 0) {
        spdlog::debug("YOU DIED");
        return true;
    }
    return false;
}

bool kill_player()
{
    uintptr_t hp_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, pointer_offsets::hp);
    int32_t hp = read_value<int32_t>(hp_ptr);
    if (hp > 0) {
        write_value<int32_t>(hp_ptr, 0);
        return true;
    }
    return false;
}

bool is_player_ingame()
{
    uintptr_t state_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, pointer_offsets::game_state);
    int32_t state = read_value<int32_t>(state_ptr);;
    if (state == 30) {
        return true;
    }
    return false;
}