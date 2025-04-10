#include "randomizer.h"

#include "offsets.h"
#include "memory.h"
#include "ds2.h"

#include "spdlog/spdlog.h"

#include <iostream>

#ifdef _M_IX86
struct ParamRow {
    uint32_t param_id;
    uint32_t reward_offset;
    uint32_t unknown;
};
#elif defined(_M_X64)
struct ParamRow {
    uint8_t padding1[4];
    uint32_t param_id;
    uint8_t padding2[4];
    uint32_t reward_offset;
    uint8_t padding3[4];
    uint32_t unknown;
};
#endif

void override_itemlot_param(std::map<int32_t, int32_t> rewards, std::vector<uintptr_t> param_offsets)
{
    uintptr_t param_ptr = resolve_pointer(get_base_address(), PointerOffsets::BaseA, param_offsets);
    int first_row_offset = 0x44 - sizeof(uintptr_t); // 0x3C for x64 and 0x40 for x40
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + first_row_offset);
    for (int i = 0; i < 10000; ++i) {
        int param_id = row_ptr[i].param_id;

        if (i > 0 && param_id == 0) return; // return if we reach the end
        if (!rewards.contains(param_id)) continue;
        if (is_shop_location(param_id)) continue; // shitty fix

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;

        for (int j = 0; j < 8; j++) {
            uintptr_t item_ptr = reward_ptr + 0x2C + (j * sizeof(uint32_t));
            uintptr_t amount_ptr = reward_ptr + 0x4 + j;
            if (j == 0) {
                write_value<uint32_t>(item_ptr, rewards[param_id]);
                write_value<uint32_t>(amount_ptr, 1);
            }
            else {
                write_value<uint32_t>(item_ptr, 10);
                write_value<uint32_t>(amount_ptr, 0);
            }
        }
    }
}

void override_item_prices()
{
    uintptr_t param_ptr = resolve_pointer(get_base_address(), PointerOffsets::BaseA, ParamOffsets::ItemParam);
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + 0x44 - sizeof(uintptr_t)); // 0x3C for x64 and 0x40 for x40

    for (int i = 0; i < 10000; ++i) {
        int param_id = row_ptr[i].param_id;
        if (i > 0 && param_id == 0) return;

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;

        uintptr_t price_ptr = reward_ptr + 0x30;
        write_value<uint32_t>(price_ptr, 1);
    
        // this sets the item type id of unused items to "Weapon Resion/Ooze" so that their icon is all the same
        // this should definetly be changed to another function, im just lazy
        auto it = std::find(unused_item_ids.begin(), unused_item_ids.end(), param_id);
        if (it != unused_item_ids.end()) {
            write_value<uint32_t>(reward_ptr + 0x40, 600);
        }
    }
}

void override_shoplineup_param(std::map<int32_t, int32_t> rewards)
{
#ifdef _M_IX86
    // for some reason in vanilla when you go through the start menu
    // they always override the params with the defaults
    // this patch makes the game not override the ShopLineupParam (and maybe others?)
    // this doesnt happen in sotfs
    patch_memory(get_base_address() + 0x316A9F, {0x90, 0x90, 0x90, 0x90, 0x90});
#endif

    // set all items base prices to 1 so that we can
    // set the values properly in the shop params
    override_item_prices();

    uintptr_t param_ptr = resolve_pointer(get_base_address(), PointerOffsets::BaseA, ParamOffsets::ShopLineupParam);
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + 0x44 - sizeof(uintptr_t)); // 0x3C for x64 and 0x40 for x40

    for (int i = 0; i < 10000; ++i) {
        int param_id = row_ptr[i].param_id;

        if (i > 0 && param_id == 0) return; // return if we reach the end

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;

        // we need to set the price rate no matter what
        // because we set the default price of all the items to 1
        uintptr_t price_rate_ptr = reward_ptr + 0x1C;
        write_value<float_t>(price_rate_ptr, get_shop_price(param_id));

        if (!rewards.contains(param_id)) continue;

        write_value<uint32_t>(reward_ptr, rewards[param_id]);

        // make it so the items never dissapear from the shop
        // to prevent the player from losing checks
        uintptr_t disable_flag_ptr = reward_ptr + 0xC;
        write_value<float_t>(disable_flag_ptr, -1.0f);

        uintptr_t amount_ptr = reward_ptr + 0x20;
        write_value<uint8_t>(amount_ptr, 1);
    }
}

void override_item_params(std::map<int32_t, int32_t> rewards)
{
    override_itemlot_param(rewards, ParamOffsets::ItemLotParam2_Other);
    override_itemlot_param(rewards, ParamOffsets::ItemLotParam2_Chr);
    override_shoplineup_param(rewards);
}