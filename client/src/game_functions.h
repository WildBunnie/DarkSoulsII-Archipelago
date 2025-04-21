#pragma once

#include <cstdint>
#include <vector>

struct Item {
    int32_t idk;
    int32_t item_id;
    int32_t durability;
    int16_t amount;
    int8_t upgrade;
    int8_t infusion;
};

struct ItemStruct {
    Item items[8];
};

typedef char(__thiscall* add_items_to_inventory_t) (uintptr_t, ItemStruct*, int32_t, int32_t);
typedef void(__cdecl*    create_popup_structure_t) (uintptr_t, ItemStruct*, int32_t, int32_t);
typedef void(__thiscall* show_item_popup_t)        (uintptr_t, uintptr_t);

void give_items(ItemStruct items, int num_items);
bool is_statue(int item_id);
void unpetrify_statue(int statue_id);

bool player_just_died();
bool kill_player();
bool is_player_ingame();