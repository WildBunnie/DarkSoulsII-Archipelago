#pragma once

#include "game_functions.h"
#include "archipelago.h"

#include <windows.h>
#include <ws2def.h>
#include <set>
#include <WS2tcpip.h>
#include <cstdint>
#include <map>
#include <list>
#include <string>

// taken from modengine
// https://github.com/rainergeis/ModEngine-DS2-Compatible/blob/master/DS3ModEngine/ModLoader.h
typedef struct {
    wchar_t* string;
    void* unk;
    UINT64 length;
    UINT64 capacity;
} DLString;

void init_hooks(std::map<int32_t, APLocation> location_map, bool autoequip);
void force_offline();
std::list<int32_t> get_locations_to_check();
void clear_locations_to_check();
void set_item_name(int32_t item_id, std::wstring item_name);

extern "C" int __cdecl get_pickup_id(uintptr_t param_1, uintptr_t baseAddress);

typedef INT(__stdcall* getaddrinfo_t)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);

typedef void (__thiscall *add_item_to_inventory_t)(uintptr_t, Item*);
typedef void (__thiscall *give_items_on_reward_t) (uintptr_t, uintptr_t, int32_t, int32_t, int32_t);
typedef char (__thiscall *give_items_on_pickup_t) (uintptr_t, uintptr_t);
typedef char (__thiscall *give_shop_item_t)       (uintptr_t, uintptr_t, int32_t);

typedef int  (__thiscall *remove_item_from_inventory_t) (uintptr_t, uintptr_t, uintptr_t, int32_t);

typedef const wchar_t* (__cdecl *get_item_info_t)             (int32_t, int32_t);
typedef uintptr_t      (__thiscall *get_hovering_item_info_t) (uintptr_t, uintptr_t);

typedef size_t(__thiscall* virtual_to_archive_path_t)(uintptr_t, DLString*);

#define HOOKS \
    HOOK(add_item_to_inventory) \
    HOOK(give_items_on_pickup) \
    HOOK(give_items_on_reward) \
    HOOK(give_shop_item) \
    HOOK(remove_item_from_inventory) \
    HOOK(get_item_info) \
    HOOK(get_hovering_item_info) \
    HOOK(virtual_to_archive_path)