#include "randomizer.h"

#include "offsets.h"
#include "memory.h"
#include "ds2.h"

#include "spdlog/spdlog.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

#ifdef _M_IX86
struct ParamRow {
    int32_t param_id;
    int32_t reward_offset;
    int32_t unknown;
};
#elif defined(_M_X64)
struct ParamRow {
    uint8_t padding1[4];
    int32_t param_id;
    uint8_t padding2[4];
    int32_t reward_offset;
    uint8_t padding3[4];
    int32_t unknown;
};
#endif

void override_itemlot_param(std::map<int32_t, int32_t> rewards, std::string seed_str, std::set<int32_t> ignore, std::vector<uintptr_t> param_offsets)
{
    uintptr_t param_ptr = resolve_pointer(get_base_address(), PointerOffsets::BaseA, param_offsets);
    int first_row_offset = 0x44 - sizeof(uintptr_t); // 0x3C for x64 and 0x40 for x40
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + first_row_offset);

    std::vector<int32_t> guaranteed_items_offsets;
    std::vector<int32_t> random_items_offsets;
    std::set<int32_t> guaranteed_items_params;
    std::set<int32_t> random_items_params;

    for (int i = 0; i < 10000; ++i) {
        int param_id = row_ptr[i].param_id;

        if (i > 0 && param_id == 0) break; // return if we reach the end
        if (ignore.contains(param_id)) continue;
        if (rewards.contains(param_id) && !shop_prices.contains(param_id)) continue;

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;

        // chance for the first item to drop
        uintptr_t chance_ptr = reward_ptr + 0x54; 
        float_t chance = read_value<float_t>(chance_ptr);

        if (chance == 100.0) {
            guaranteed_items_offsets.push_back(row_ptr[i].reward_offset);
            guaranteed_items_params.insert(param_id);
        }
        else {
            random_items_offsets.push_back(row_ptr[i].reward_offset);
            random_items_params.insert(param_id);
        }
    }

    std::hash<std::string> hasher;
    size_t seed = hasher(seed_str);

    std::default_random_engine rng(static_cast<unsigned int>(seed));
    std::ranges::shuffle(guaranteed_items_offsets, rng);
    std::ranges::shuffle(random_items_offsets, rng);

    for (int i = 0; i < 10000; ++i) {
        int param_id = row_ptr[i].param_id;

        if (i > 0 && param_id == 0) break; // return if we reach the end
        if (ignore.contains(param_id)) continue;

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;
        if (rewards.contains(param_id) && !shop_prices.contains(param_id)) {
            uintptr_t item_ptr = reward_ptr + 0x2C;
            write_value<int32_t>(item_ptr, rewards[param_id]);

            uintptr_t amount_ptr = reward_ptr + 0x4;
            write_value<int8_t>(amount_ptr, 1);

            // zero out the amounts of the other items
            void* ptr = reinterpret_cast<void*>(amount_ptr + 1);
            std::memset(ptr, 0, 9 * sizeof(int8_t));
        }
        else {
            uintptr_t offset = row_ptr[i].reward_offset;
            if (guaranteed_items_params.contains(param_id)) {
                offset = guaranteed_items_offsets.back();
                guaranteed_items_offsets.pop_back();
            }
            else if (random_items_params.contains(param_id)) {
                offset = random_items_offsets.back();
                random_items_offsets.pop_back();
            }
            row_ptr[i].reward_offset = offset;
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
        write_value<int32_t>(price_ptr, 1);
    
        // this sets the item type id of unused items to "Weapon Resion/Ooze" so that their icon is all the same
        // this should definetly be changed to another function, im just lazy
        auto it = std::find(unused_item_ids.begin(), unused_item_ids.end(), param_id);
        if (it != unused_item_ids.end()) {
            write_value<int32_t>(reward_ptr + 0x40, 600);
        }
    }
}

void override_shoplineup_param(std::map<int32_t, int32_t> rewards, std::string seed_str, std::set<int32_t> ignore)
{
    // set all items base prices to 1 so that we can
    // set the values properly in the shop params
    override_item_prices();

    uintptr_t param_ptr = resolve_pointer(get_base_address(), PointerOffsets::BaseA, ParamOffsets::ShopLineupParam);
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + 0x44 - sizeof(uintptr_t)); // 0x3C for x64 and 0x40 for x40

    typedef struct {
        int32_t id;
        uint8_t amount;
    } ItemWithAmount;

    std::vector<int32_t> unique_items;
    std::vector<ItemWithAmount> non_unique_items;
    std::vector<int32_t> infinite_items;

    // get all the items for the static randomizer
    for (int i = 0; i < 10000; ++i) {
        int param_id = row_ptr[i].param_id;

        if (i > 0 && param_id == 0) break; // return if we reach the end
        if (ignore.contains(param_id)) continue;
        if (rewards.contains(param_id)) continue; // we deal with predefined items in the next for loop
        if (param_id >= 76801000 && param_id <= 76801306) continue; // skip straid trades
        if (param_id >= 77601000 && param_id <= 77602121) continue; // skip ornifex trades

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;
        uintptr_t amount_ptr = reward_ptr + 0x20;
        int32_t item_id = read_value<int32_t>(reward_ptr);
        uint8_t amount = read_value<uint8_t>(amount_ptr);

        if (item_id == 0 || amount == 0) continue;

        if (amount == 1) {
            //spdlog::debug("unique item {}", param_id);
            unique_items.push_back(item_id);
        }
        else if (amount == 255) {
            infinite_items.push_back(item_id);
        }
        else {
            ItemWithAmount item = { item_id, amount };
            non_unique_items.push_back(item);
        }
    }

    std::hash<std::string> hasher;
    size_t seed = hasher(seed_str);

    std::default_random_engine rng(static_cast<unsigned int>(seed));
    std::ranges::shuffle(unique_items, rng);
    std::ranges::shuffle(non_unique_items, rng);
    std::ranges::shuffle(infinite_items, rng);

    for (int i = 0; i < 10000; ++i) {
        int param_id = row_ptr[i].param_id;

        if (i > 0 && param_id == 0) break; // return if we reach the end

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;
        uintptr_t amount_ptr = reward_ptr + 0x20;

        // we need to set the price rate no matter what
        // because we set the default price of all the items to 1
        if (shop_prices.contains(param_id)) {
            uintptr_t price_rate_ptr = reward_ptr + 0x1C;
            write_value<float_t>(price_rate_ptr, shop_prices.at(param_id));
        }

        // make it so the items never dissapear from the shop
        // to prevent the player from losing checks
        uintptr_t disable_flag_ptr = reward_ptr + 0xC;
        write_value<float_t>(disable_flag_ptr, -1.0f);

        if (ignore.contains(param_id)) continue;
        if (param_id >= 76801000 && param_id <= 76801306) continue; // skip straid trades
        if (param_id >= 77601000 && param_id <= 77602121) continue; // skip ornifex trades

        if (rewards.contains(param_id)) {
            write_value<int32_t>(reward_ptr, rewards[param_id]);
            write_value<uint8_t>(amount_ptr, 1);
        }
        else {
            int32_t item_id = read_value<int32_t>(reward_ptr);
            uint8_t amount = read_value<uint8_t>(amount_ptr);
            if (item_id == 0 || amount == 0) continue;

            if (amount == 1) {
                item_id = unique_items.back();
                unique_items.pop_back();
            }   
            else if (amount == 255) {
                item_id = infinite_items.back();
                infinite_items.pop_back();
            }
            else {
                ItemWithAmount item = non_unique_items.back();
                non_unique_items.pop_back();

                item_id = item.id;
                write_value<uint8_t>(amount_ptr, item.amount);
            }
            write_value<int32_t>(reward_ptr, item_id);
        }
    }
}

void override_item_params(std::map<int32_t, int32_t> rewards, std::string seed, std::set<int32_t> ignore)
{
    using namespace std::chrono;

    spdlog::info("Randomizing items...");
    auto start_time = high_resolution_clock::now();

#ifdef _M_IX86
    // for some reason in vanilla when you go through the start menu
    // they always override the params with the defaults
    // this patch makes the game not override params
    // this doesnt happen in sotfs
    patch_memory(get_base_address() + 0x316A9F, { 0x90, 0x90, 0x90, 0x90, 0x90 });
#endif

    override_itemlot_param(rewards, seed, ignore, ParamOffsets::ItemLotParam2_Other);
    override_itemlot_param(rewards, seed, ignore, ParamOffsets::ItemLotParam2_Chr);
    override_shoplineup_param(rewards, seed, ignore);

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    spdlog::info("Randomization complete!");
    spdlog::debug("randomization finished in {} ms.", duration.count());
}