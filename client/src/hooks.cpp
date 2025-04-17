#include "hooks.h"

#include "offsets.h"
#include "memory.h"
#include "game_functions.h"
#include "ds2.h"

#include "spdlog/spdlog.h"
#include "minhook.h"

#include <map>
#include <cwctype>

getaddrinfo_t original_getaddrinfo;

#define HOOK(name) name##_t original_##name;
HOOKS
#undef HOOK

bool hooks_enabled = false;
std::map<int, std::wstring> item_names;
std::map<int32_t, std::string> _reward_names;
std::map<int32_t, int32_t> _custom_items;
std::list<int32_t> locations_to_check;

std::wstring remove_special_characters(const std::wstring& input)
{
    std::set<wchar_t> allowedChars = {
        L'-', L'\'', L',', L' ', L':', L'+'
    };

    std::wstring output;
    for (wchar_t ch : input) {
        // Keep the character if it's alphanumeric or in the allowed special characters list
        if (std::iswalnum(ch) || allowedChars.find(ch) != allowedChars.end()) {
            output += ch;
        }
    }
    return output;
}

void handle_location_checked(int32_t location_id)
{
    // if the location has an item from other player
    // set the item to have the correct name
    if (_reward_names.contains(location_id)) {
        std::string item_name = _reward_names[location_id];
        std::wstring item_name_wide(item_name.begin(), item_name.end());
        item_names[the_item_id] = remove_special_characters(item_name_wide);
    }
    // if its a custom item, do whatever we need to do
    if (_custom_items.contains(location_id)) {
        int32_t item_id = _custom_items[location_id];
        if (is_statue(item_id)) {
            unpetrify_statue(item_id);
        }
    }
    locations_to_check.push_back(location_id);
}

INT __stdcall detour_getaddrinfo(PCSTR address, PCSTR port, const ADDRINFOA* pHints, PADDRINFOA* ppResult)
{
#ifdef _M_IX86
    const char* address_to_block = "frpg2-steam-ope.fromsoftware.jp";
#elif defined(_M_X64)
    const char* address_to_block = "frpg2-steam64-ope-login.fromsoftware-game.net";
#endif

    if (address && strcmp(address, address_to_block) == 0) {
        return EAI_FAIL;
    }

    return original_getaddrinfo(address, port, pHints, ppResult);
}

#ifdef _M_IX86
void __fastcall detour_give_items_on_reward(uintptr_t param_1, void* _edx, uintptr_t param_2, int32_t param_3, int32_t param_4, int32_t param_5)
#elif defined(_M_X64)
void __cdecl detour_give_items_on_reward(uintptr_t param_1, uintptr_t param_2, int32_t param_3, int32_t param_4, int32_t param_5)
#endif
{
    int32_t itemlot_id = read_value<int32_t>(param_2);
    spdlog::debug("was rewarded: {}", itemlot_id);
    handle_location_checked(itemlot_id);
    return original_give_items_on_reward(param_1, param_2, param_3, param_4, param_5);
}

#ifdef _M_IX86
char __fastcall detour_give_items_on_pickup(uintptr_t param_1, void* _edx, uintptr_t param_2)
#elif defined(_M_X64)
char __cdecl detour_give_items_on_pickup(uintptr_t param_1, uintptr_t param_2)
#endif
{
    int32_t itemlot_id = get_pickup_id(param_2, get_base_address());
    spdlog::debug("picked up: {}", itemlot_id);

    // janky way to fix the problem that some pickups
    // have the same id as some of the shop items
    if (!shop_prices.contains(itemlot_id)) {
        handle_location_checked(itemlot_id);
    }

    return original_give_items_on_pickup(param_1, param_2);
}

#ifdef _M_IX86
char __fastcall detour_give_shop_item(uintptr_t param_1, void* _edx, uintptr_t param_2, int32_t param_3)
#elif defined(_M_X64)
char __cdecl detour_give_shop_item(uintptr_t param_1, uintptr_t param_2, int32_t param_3)
#endif
{
    uint32_t offset = sizeof(uintptr_t) == 4 ? 0x8 : 0x10; // 0x8 in x32 and 0x10 in x64
    int32_t shop_lineup_id = read_value<int32_t>(param_2 + offset);
    spdlog::debug("just bought: {}", shop_lineup_id);
    handle_location_checked(shop_lineup_id);
    return original_give_shop_item(param_1, param_2, param_3);
}

const wchar_t* __cdecl detour_get_item_info(int32_t flag, int32_t item_id)
{
    if (item_names.contains(item_id)) {
        if (flag == 8) {
            return item_names[item_id].c_str();
        }
        else if (flag == 9) {
            return L"Archipelago Item";
        }
    }

    // rename "Pharros' Lockstone" to "Master Lockstone"
    if (item_id == 60536000) {
        if (flag == 8) {
            return L"Master Lockstone";
        }
        else if (flag == 9) {
            return L"Activates Pharros' contraptions (unlimited uses)";
        }
    }

    return original_get_item_info(flag, item_id);
}

#ifdef _M_IX86
uintptr_t __fastcall detour_get_hovering_item_info(uintptr_t param_1, void* _edx, uintptr_t param_2)
#elif defined(_M_X64)
uintptr_t __cdecl detour_get_hovering_item_info(uintptr_t param_1, uintptr_t param_2)
#endif
{
    int offset = sizeof(uintptr_t) * 2; // 0x8 in x86 and 0x10 in x64
    uintptr_t ptr = read_value<uintptr_t>(param_1 + offset);
    if (ptr != 0) {
        uint32_t itemlot_id = read_value<uint32_t>(ptr + offset);
        if (_reward_names.contains(itemlot_id)) {
            std::string item_name = _reward_names[itemlot_id];
            std::wstring item_name_wide(item_name.begin(), item_name.end());
            item_names[the_item_id] = remove_special_characters(item_name_wide);
        }
    }
    return original_get_hovering_item_info(param_1, param_2);
}

#ifdef _M_IX86
int32_t __fastcall detour_remove_item_from_inventory(uintptr_t param_1, void* _edx, uintptr_t param_2, uintptr_t inventory_item_ptr, uint32_t amount_to_remove)
#elif defined(_M_X64)
int32_t __cdecl detour_remove_item_from_inventory(uintptr_t param_1, uintptr_t param_2, uintptr_t inventory_item_ptr, uint32_t amount_to_remove)
#endif
{
    uint32_t item_id_offset = sizeof(uintptr_t) == 4 ? 0xC : 0x18; // 0xC in x86 and 0x18 in x64
    uint32_t amount_offset = sizeof(uintptr_t) == 4 ? 0x18 : 0x20; // 0x18 in x86 and 0x20 in x64

    uint32_t item_id = read_value<int32_t>(inventory_item_ptr + item_id_offset);
    uint32_t current_amount = read_value<int32_t>(inventory_item_ptr + amount_offset);

    if (amount_to_remove <= current_amount && item_id == 60536000) {
        spdlog::debug("used a Pharros' Lockstone");
        return 0;
    }

    return original_remove_item_from_inventory(param_1, param_2, inventory_item_ptr, amount_to_remove);
}

void init_hooks(std::map<int32_t, std::string> reward_names, std::map<int32_t, int32_t> custom_items)
{
    uintptr_t base_address = get_base_address();

    _reward_names = reward_names;
    _custom_items = custom_items;

    if (hooks_enabled) return;

    MH_Initialize();

    #define HOOK(name) \
    if (MH_CreateHook((LPVOID)(base_address + function_offsets::name), &detour_##name, (LPVOID*)&original_##name) != MH_OK) { \
        spdlog::error("error creating hook {}", #name); \
    } \
    else if (MH_EnableHook((LPVOID)(base_address + function_offsets::name)) != MH_OK) { \
        spdlog::error("error enabling hook {}", #name); \
    } 
    HOOKS
#undef HOOK

    hooks_enabled = true;
}

void force_offline()
{
    uintptr_t base_address = get_base_address();

    MH_Initialize();
    LPVOID target = nullptr;
    MH_CreateHookApiEx(L"ws2_32", "getaddrinfo", &detour_getaddrinfo, (LPVOID*)&original_getaddrinfo, &target);
    MH_EnableHook(target);
}

std::list<int32_t> get_locations_to_check()
{
    return locations_to_check;
}

void clear_locations_to_check()
{
    locations_to_check.clear();
}

void set_item_name(int32_t item_id, std::wstring item_name)
{
    item_names[item_id] = item_name;
}