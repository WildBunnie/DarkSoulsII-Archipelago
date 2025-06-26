#pragma once

#include "game_functions.h"

#include <windows.h>
#include <ws2def.h>
#include <set>
#include <WS2tcpip.h>
#include <cstdint>
#include <map>
#include <list>
#include <string>

void init_hooks(std::map<int32_t, std::string> reward_names, std::map<int32_t, int32_t> custom_items, bool autoequip);
std::list<int32_t> get_locations_to_check();
void clear_locations_to_check();
void set_item_name(int32_t item_id, std::wstring item_name);

extern "C" int __cdecl get_pickup_id(uintptr_t param_1, uintptr_t baseAddress);

typedef void (__thiscall *add_item_to_inventory_t)(uintptr_t, Item*);
typedef void (__thiscall *give_items_on_reward_t) (uintptr_t, uintptr_t, int32_t, int32_t, int32_t);
typedef char (__thiscall *give_items_on_pickup_t) (uintptr_t, uintptr_t);
typedef char (__thiscall *give_shop_item_t)       (uintptr_t, uintptr_t, int32_t);

typedef int  (__thiscall *remove_item_from_inventory_t) (uintptr_t, uintptr_t, uintptr_t, int32_t);

typedef const wchar_t* (__cdecl *get_item_info_t)             (int32_t, int32_t);
typedef uintptr_t      (__thiscall *get_hovering_item_info_t) (uintptr_t, uintptr_t);


#define HOOKS \
    HOOK(add_item_to_inventory) \
    HOOK(give_items_on_pickup) \
    HOOK(give_items_on_reward) \
    HOOK(give_shop_item) \
    HOOK(remove_item_from_inventory) \
    HOOK(get_item_info) \
    HOOK(get_hovering_item_info)