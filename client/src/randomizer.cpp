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

struct PlayerStatus {
    int8_t idc1[36]; // body,hollow and build type
    int32_t items[10];
    uint16_t item_amounts[10];
    int32_t spells[10];
    int32_t right_weapons[3];
    int32_t left_weapons[3];
    int32_t head_armor;
    int32_t chest_armor;
    int32_t hands_armor;
    int32_t legs_armor;
    int32_t rings[4];
    uint32_t reinforcements[6];
    int32_t unk04;
    int32_t player_status_items[3];
    int32_t unk05;
    int32_t arrows[2];
    int32_t bolts[2];
    int16_t arrow_amounts[2];
    int16_t bolt_amounts[2];
    int32_t gestures[8];
    int8_t idc2[4];
};

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

void override_itemlot_param(std::map<int32_t, int32_t> rewards, std::string seed_str, std::set<int32_t> ignore, std::vector<uintptr_t> param_offsets)
{
    uintptr_t param_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets);
    int first_row_offset = 0x44 - sizeof(uintptr_t); // 0x3C for x64 and 0x40 for x86
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
        
        uintptr_t item_ptr = reward_ptr + 0x2C;
        int32_t item_id = read_value<int32_t>(item_ptr);

        if (item_id == 60510000 || item_id == 0) {
            ignore.insert(param_id);
            continue;
        }

        // chance for the first item to drop
        uintptr_t chance_ptr = reward_ptr + 0x54; 
        float_t chance = read_value<float_t>(chance_ptr);

        if (chance == 100.0) {
            guaranteed_items_offsets.push_back(row_ptr[i].reward_offset);
            guaranteed_items_params.insert(param_id);
        }
        // ignore stupid itemlots that have the first item with 
        // 0% chance and the second item with 100% chance
        else if (chance == 0.0) {
            ignore.insert(param_id);
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

            // zero out the reinforcement level
            ptr = reinterpret_cast<void*>(amount_ptr + 0xA);
            std::memset(ptr, 0, 10 * sizeof(int8_t));

            // zero out the infusions
            ptr = reinterpret_cast<void*>(amount_ptr + 0x14);
            std::memset(ptr, 0, 10 * sizeof(int8_t));
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

void override_shoplineup_param(std::map<int32_t, int32_t> rewards, std::string seed_str, std::set<int32_t> ignore)
{
    // set all items base prices to 1 so that we can
    // set the values properly in the shop params
    override_item_prices();

    uintptr_t param_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::shop_lineup_param);
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + 0x44 - sizeof(uintptr_t)); // 0x3C for x64 and 0x40 for x86

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

    patch_lockstone_stuff();

    override_itemlot_param(rewards, seed, ignore, param_offsets::item_lot_param2_other);
    override_itemlot_param(rewards, seed, ignore, param_offsets::item_lot_param2_chr);
    override_shoplineup_param(rewards, seed, ignore);

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