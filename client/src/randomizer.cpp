#include "randomizer.h"

#include "offsets.h"
#include "memory.h"
#include "ds2.h"
#include "params.h"

#include "spdlog/spdlog.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

// make it so locksones are not sellable
// and make it so they dont drop from rats
void patch_lockstone_stuff()
{
    uintptr_t base_address = get_base_address();

    uintptr_t item_param = resolve_pointer(base_address, pointer_offsets::base_a, param_offsets::item_param);
    uint32_t offset = sizeof(uintptr_t) == 4 ? 0x1AB20 : 0x1E984; // 0x1AB20 in x32 and 0x1E984 in x64
    write_value<int32_t>(item_param + offset, 2240); // set it so lockstones have the same usage param as branches

    uintptr_t itemlot_chr_param = resolve_pointer(base_address, pointer_offsets::base_a, param_offsets::item_lot_param2_chr);
    offset = sizeof(uintptr_t) == 4 ? 0xAAD8 : 0x10A7C; // 0xAAD8 in x32 and 0x10A7C in x64
    write_value<float_t>(itemlot_chr_param + offset, 0.0f); // make rats have 0% chance to drop lockstone
}

void override_itemlot_param(std::map<int32_t, APLocation> location_map, std::string seed_str, std::set<int32_t> ignore, std::vector<uintptr_t> param_offsets)
{
    std::vector<int32_t> guaranteed_items_offsets;
    std::vector<int32_t> random_items_offsets;
    std::set<int32_t> guaranteed_items_params;
    std::set<int32_t> random_items_params;

    uintptr_t table_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets);
    ParamTable* table = (ParamTable*)table_ptr;
    
    LocationType type;
    if (param_offsets == param_offsets::item_lot_param2_other) {
        type = ItemLotParam2_Other_Location;
    }
    else if (param_offsets == param_offsets::item_lot_param2_chr) {
        type = ItemLotParam2_Chr_Location;
    }
    else {
        assert(false && "could not find proper location type");
    }

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];
        ITEM_LOT_PARAM2* param = (ITEM_LOT_PARAM2*)(table_ptr + row->reward_offset);
    
        int32_t location_id = row->param_id + get_location_offset(type);
        if (ignore.contains(location_id)) continue;
        if (location_map.contains(location_id)) continue; // we deal with predefined items in the next for loop

        if (param->chance_lot_0 == 100.0) {
            guaranteed_items_offsets.push_back(row->reward_offset);
            guaranteed_items_params.insert(row->param_id);
        }
        else if (param->chance_lot_0 > 0.0){
            random_items_offsets.push_back(row->reward_offset);
            random_items_params.insert(row->param_id);
        }
    }
    
    std::hash<std::string> hasher;
    size_t seed = hasher(seed_str);
    
    std::default_random_engine rng(static_cast<unsigned int>(seed));
    std::ranges::shuffle(guaranteed_items_offsets, rng);
    std::ranges::shuffle(random_items_offsets, rng);

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];

        int32_t param_id = row->param_id;
        ITEM_LOT_PARAM2* param = (ITEM_LOT_PARAM2*)(table_ptr + row->reward_offset);

        int32_t location_id = param_id + get_location_offset(type);
        if (ignore.contains(location_id)) continue;

        if (location_map.contains(location_id)) {
            APLocation& location = location_map[location_id];

            int32_t* items = &param->item_lot_0;
            uint8_t* reinforcements = &param->reinforcement_lot_0;
            uint8_t* infusions = &param->infusion_lot_0;
            uint8_t* amounts = &param->amount_lot_0;
            float_t* chances = &param->chance_lot_0;

            // zero out to guarantee nothing weird happens
            std::memset(items, 0, sizeof(int32_t) * 10);
            std::memset(reinforcements, 0, sizeof(int8_t) * 10);
            std::memset(infusions, 0, sizeof(int8_t) * 10);
            std::memset(amounts, 0, sizeof(int8_t) * 10);
            std::memset(chances, 0, sizeof(float_t) * 10);

            for (int j = 0; j < location.reward_amount && j < 10; j++) {
                APLocationReward& reward = location.rewards[j];
                items[j] = reward.real_item_id;
                amounts[j] = 1;

                // convert bundle item id to the item with the amount
                if (get_item_bundles().contains(reward.real_item_id)) {
                    int bundle_amount = reward.real_item_id % 1000;
                    items[j] = reward.real_item_id - bundle_amount;
                    amounts[j] = bundle_amount;
                }
                
                chances[j] = 100.0f;
            }
        }
        else {
            uintptr_t offset = row->reward_offset;
            if (guaranteed_items_params.contains(param_id)) {
                offset = guaranteed_items_offsets.back();
                guaranteed_items_offsets.pop_back();
            }
            else if (random_items_params.contains(param_id)) {
                offset = random_items_offsets.back();
                random_items_offsets.pop_back();
            }
            row->reward_offset = offset;
        }
    }
}

void override_item_prices()
{
    uintptr_t param_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::item_param);
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + 0x44 - sizeof(uintptr_t)); // 0x3C for x64 and 0x40 for x86

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

bool is_valid_shop(int32_t param_id)
{
    if (param_id == 0) return false;
    if (param_id >= 76801000 && param_id <= 76801306) return false; // skip straid trades
    if (param_id >= 77601000 && param_id <= 77602121) return false; // skip ornifex trades
    return true;
}

void override_shoplineup_param(std::map<int32_t, APLocation> location_map, std::string seed_str, std::set<int32_t> ignore)
{
    // set all items base prices to 1 so that we can
    // set the values properly in the shop params
    override_item_prices();

    struct ItemWithAmount {
        int32_t id;
        uint8_t amount;
    };

    std::vector<int32_t> unique_items;
    std::vector<ItemWithAmount> non_unique_items;
    std::vector<int32_t> infinite_items;

    uintptr_t table_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::shop_lineup_param);
    ParamTable* table = (ParamTable*)table_ptr;

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];
        SHOP_LINEUP_PARAM* param = (SHOP_LINEUP_PARAM*)(table_ptr + row->reward_offset);

        if (!is_valid_shop(row->param_id)) continue;
        int32_t location_id = row->param_id + get_location_offset(ShopLineupParam_Location);
        if (ignore.contains(location_id)) continue;
        if (location_map.contains(location_id)) continue; // we deal with predefined items in the next for loop
        
        if (param->quantity == 1) {
            unique_items.push_back(param->item_id);
        }
        else if (param->quantity == 255) {
            infinite_items.push_back(param->item_id);
        }
        else {
            ItemWithAmount item = { param->item_id, param->quantity };
            non_unique_items.push_back(item);
        }
    }

    std::hash<std::string> hasher;
    size_t seed = hasher(seed_str);

    std::default_random_engine rng(static_cast<unsigned int>(seed));
    std::ranges::shuffle(unique_items, rng);
    std::ranges::shuffle(non_unique_items, rng);
    std::ranges::shuffle(infinite_items, rng);

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];

        int32_t param_id = row->param_id;
        SHOP_LINEUP_PARAM* param = (SHOP_LINEUP_PARAM*)(table_ptr + row->reward_offset);

        // since we set all items to price = 1 above we need to set price rate on all rows
        if (shop_prices.contains(param_id)) {
            param->price_rate = shop_prices.at(param_id); 
        }

        if (!is_valid_shop(row->param_id)) continue;

        int32_t location_id = param_id + get_location_offset(ShopLineupParam_Location);
        if (ignore.contains(location_id)) continue;

        if (location_map.contains(location_id)) {
            APLocation& location = location_map[location_id];
            assert(location.reward_amount == 1 && "shop location must contain exactly 1 reward");

            param->item_id = location.rewards[0].real_item_id;
            param->disable_flag = -1;
            param->quantity = 1;
            param->price_rate = shop_prices.at(param_id);
        }
        else {
            if (param->item_id == 0) continue;

            if (param->quantity == 1) {
                param->item_id = unique_items.back();
                unique_items.pop_back();
            }   
            else if (param->quantity == 255) {
                param->item_id = infinite_items.back();
                infinite_items.pop_back();
            }
            else {
                ItemWithAmount item = non_unique_items.back();
                non_unique_items.pop_back();

                param->item_id = item.id;
                param->quantity = item.amount;
            }
        }
    }
}

void override_item_params(std::map<int32_t, APLocation> location_map, std::string seed, std::set<int32_t> ignore)
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

    patch_lockstone_stuff();

    override_itemlot_param(location_map, seed, ignore, param_offsets::item_lot_param2_other);
    override_itemlot_param(location_map, seed, ignore, param_offsets::item_lot_param2_chr);
    override_shoplineup_param(location_map, seed, ignore);

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    spdlog::info("Randomization complete!");
    spdlog::debug("randomization finished in {} ms.", duration.count());
}

enum StarterClass {
    Warrior = 1,
    Knight = 2,
    Bandit = 3,
    Cleric = 4,
    Sorcerer = 5,
    Explorer = 6,
    Swordsman = 7,
    Deprived = 8
};

int32_t get_valid_random_item(std::string seed_str, std::set<int32_t> items, std::map<int32_t, StatBlock> item_requirements, StatBlock current_stats)
{
    std::hash<std::string> hasher;
    size_t seed = hasher(seed_str);
    std::default_random_engine rng(static_cast<unsigned int>(seed));

    std::vector<int32_t> valid_items;
    for (auto item_id : items) {
        if (item_requirements.contains(item_id)) {
            StatBlock req = item_requirements[item_id];
            if (req.str <= current_stats.str &&
                req.dex <= current_stats.dex &&
                req.intl <= current_stats.intl &&
                req.fth <= current_stats.fth)
            {
                valid_items.push_back(item_id);
            }
        }
    }

    if (valid_items.empty()) {
        spdlog::debug("No valid items found for given stats.");
        return 3400000;
    }

    std::uniform_int_distribution<size_t> dist(0, valid_items.size() - 1);
    return valid_items[dist(rng)];
}

int32_t get_random_item(std::string seed_str, std::set<int32_t> items)
{
    std::hash<std::string> hasher;
    size_t seed = hasher(seed_str);
    std::default_random_engine rng(static_cast<unsigned int>(seed));

    std::vector<int32_t> item_vec(items.begin(), items.end());
    std::uniform_int_distribution<size_t> dist(0, item_vec.size() - 1);
    return item_vec[dist(rng)];
}

int get_random_amount(std::string seed_str, int min = 1, int max = 5)
{
    std::hash<std::string> hasher;
    size_t hash = hasher(seed_str);
    std::default_random_engine rng(static_cast<unsigned int>(hash));
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

StatBlock get_class_stats(StarterClass class_id)
{
    switch (class_id) {
        case Warrior:   return { 15, 11, 5, 5 };
        case Knight:    return { 11, 8, 3, 6 };
        case Bandit:    return { 9, 14, 1, 8 };
        case Cleric:    return { 11, 5, 4, 12 };
        case Sorcerer:  return { 3, 7, 14, 4 };
        case Explorer:  return { 6, 6, 5, 5 };
        case Swordsman: return { 9, 16, 7, 5 };
        case Deprived:  return { 6, 6, 6, 6 };
        default:
            spdlog::warn("Unknown class_id {}", static_cast<int>(class_id));
            return { -1, -1, -1, -1 };
    }
}

void randomize_starter_classes(std::string seed_str, ClassRandomizationFlag flag)
{
    uintptr_t param_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::player_status_param);
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + 0x44 - sizeof(uintptr_t)); // 0x3C for x64 and 0x40 for x86

    std::map<int32_t, StatBlock> weapons = get_weapon_requirements();

    // we leave deprived non randomized
    for (int i = Warrior; i < Deprived; ++i) {
        PlayerStatus* param = reinterpret_cast<PlayerStatus*>(param_ptr + row_ptr[i].reward_offset);

        StatBlock class_stats = get_class_stats(static_cast<StarterClass>(i));
        if (flag == TWO_HANDABLE) {
            class_stats.str *= 2;
        }
        else if (flag == FULL_RANDOM) {
            class_stats.str = 1000;
            class_stats.dex = 1000;
            class_stats.intl = 1000;
            class_stats.fth = 1000;
        }

        std::string class_seed = seed_str + std::to_string(i);

        param->items[0] = get_random_item(class_seed, goods);
        param->item_amounts[0] = 7; // 7 cause the explorer has 7 items, random could also be cool

        param->right_weapons[0] = get_valid_random_item(class_seed, melee_weapons, weapons, class_stats);
        param->right_weapons[1] = 3400000;
        param->right_weapons[2] = 3400000;
        param->left_weapons[0] = 3400000;
        param->left_weapons[1] = 3400000;
        param->left_weapons[2] = 3400000;

        switch (i) {
            case Warrior: {
                param->head_armor = get_random_item(class_seed, head_armor);
                param->chest_armor = get_random_item(class_seed, chest_armor);
                param->hands_armor = get_random_item(class_seed, hands_armor);
                param->legs_armor = get_random_item(class_seed, legs_armor);

                param->left_weapons[0] = get_valid_random_item(class_seed, shields, weapons, class_stats);
                break;
            }
            case Knight: {
                param->chest_armor = get_random_item(class_seed, chest_armor);
                param->hands_armor = get_random_item(class_seed, hands_armor);
                param->legs_armor = get_random_item(class_seed, legs_armor);
                break;
            }
            case Bandit: {
                param->head_armor = get_random_item(class_seed, head_armor);
                param->chest_armor = get_random_item(class_seed, chest_armor);
                param->hands_armor = get_random_item(class_seed, hands_armor);
                param->legs_armor = get_random_item(class_seed, legs_armor);

                param->left_weapons[0] = get_valid_random_item(class_seed, bows, weapons, class_stats);
                param->arrows[0] = get_random_item(seed_str, arrows);
                param->arrow_amounts[0] = get_random_amount(seed_str, param->arrow_amounts[0] - 10, param->arrow_amounts[0] + 10);
                break;
            }
            case Cleric: {
                param->chest_armor = get_random_item(class_seed, chest_armor);

                param->left_weapons[0] = get_valid_random_item(class_seed, chimes, weapons, class_stats);

                param->spells[0] = get_random_item(class_seed, miracles);
                break;
            }
            case Sorcerer: {
                param->chest_armor = get_random_item(class_seed, chest_armor);
                param->legs_armor = get_random_item(class_seed, legs_armor);

                param->left_weapons[0] = get_valid_random_item(class_seed, staffs, weapons, class_stats);

                param->spells[0] = get_random_item(class_seed, sorceries);
                break;
            }
            case Swordsman: {
                param->head_armor = get_random_item(class_seed, head_armor);
                param->chest_armor = get_random_item(class_seed, chest_armor);
                param->hands_armor = get_random_item(class_seed, hands_armor);
                param->legs_armor = get_random_item(class_seed, legs_armor);

                param->left_weapons[0] = get_valid_random_item(class_seed, melee_weapons, weapons, class_stats);
                break;
            }
            case Explorer: {
                param->head_armor = get_random_item(class_seed, head_armor);
                param->chest_armor = get_random_item(class_seed, chest_armor);
                param->hands_armor = get_random_item(class_seed, hands_armor);
                param->legs_armor = get_random_item(class_seed, legs_armor);

                param->rings[0] = -1; // maybe i shouldnt remove his ring idk

                for (int i = 0; i < 7; i++) {
                    param->items[i] = get_random_item(class_seed + std::to_string(i), goods);
                    param->item_amounts[i] = 1;
                }
                param->bolts[0] = get_random_item(seed_str, bolts);
                param->bolt_amounts[0] = get_random_amount(seed_str, param->bolt_amounts[0] - 10, param->bolt_amounts[0] + 10);
                break;
            }
        }

        // force reinforcements to 0
        for (int i = 0; i < 6; i++)
            param->reinforcements[i] = 0;
    }

    // gifts
    for (int i = 10; i <= 16; ++i) {
        PlayerStatus* param = reinterpret_cast<PlayerStatus*>(param_ptr + row_ptr[i].reward_offset);

        std::string gift_seed = seed_str + std::to_string(i);
        if (i == 10) {
            int32_t ring = get_random_item(gift_seed + std::to_string(i), rings);
            param->rings[0] = ring - (ring % 10); // make last digit 0 so that its a +0 ring
        }
        else {
            param->items[0] = get_random_item(gift_seed + std::to_string(i), goods);
            param->item_amounts[0] = 1;
        }

        for (int i = 1; i < 10; i++) {
            param->items[i] = -1;
            param->item_amounts[i] = 0;
        }
    }
}