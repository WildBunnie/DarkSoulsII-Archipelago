#pragma once

#include <cstdint>
#include <map>

#ifdef _M_IX86
typedef struct {
    int32_t param_id;
    int32_t reward_offset;
    int32_t unknown;
} ParamRow;
#elif defined(_M_X64)
typedef struct {
    uint8_t padding1[4];
    int32_t param_id;
    uint8_t padding2[4];
    int32_t reward_offset;
    uint8_t padding3[4];
    int32_t unknown;
} ParamRow;
#endif

#ifdef _M_IX86
typedef struct {
} ParamTable;
#elif defined(_M_X64)
typedef struct {
    uint8_t unk01[10];
    uint16_t row_amount;
    char header_text[48];
    ParamRow rows[];
} ParamTable;
#endif

struct StatBlock {
    int16_t str = 0;
    int16_t dex = 0;
    int16_t intl = 0;
    int16_t fth = 0;
};

enum ItemCategory {
    Weapon = 0,
    Shield = 1,
    HeadArmor = 2,
    ChestArmor = 3,
    HandsArmor = 4,
    LegsArmor = 5,
    Ammunition = 6,
    Ring = 7,
    Good = 8,
    Spell = 9,
    Gesture = 10
};

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

typedef struct {
    int32_t item_id;
    int32_t Unk00;
    int32_t enable_flag;
    int32_t disable_flag;
    int32_t material_id;
    int32_t duplicate_item_id;
    int32_t Unk01;
    float_t price_rate;
    int32_t quantity;
} SHOP_LINEUP_PARAM;

typedef struct {
    uint8_t lots_dropped_order;
    uint8_t lots_dropped_num;
    uint8_t Unk02;
    uint8_t Unk03;
    uint8_t amount_lot_0;
    uint8_t amount_lot_1;
    uint8_t amount_lot_2;
    uint8_t amount_lot_3;
    uint8_t amount_lot_4;
    uint8_t amount_lot_5;
    uint8_t amount_lot_6;
    uint8_t amount_lot_7;
    uint8_t amount_lot_8;
    uint8_t amount_lot_9;
    uint8_t reinforcement_lot_0;
    uint8_t reinforcement_lot_1;
    uint8_t reinforcement_lot_2;
    uint8_t reinforcement_lot_3;
    uint8_t reinforcement_lot_4;
    uint8_t reinforcement_lot_5;
    uint8_t reinforcement_lot_6;
    uint8_t reinforcement_lot_7;
    uint8_t reinforcement_lot_8;
    uint8_t reinforcement_lot_9;
    uint8_t infusion_lot_0;
    uint8_t infusion_lot_1;
    uint8_t infusion_lot_2;
    uint8_t infusion_lot_3;
    uint8_t infusion_lot_4;
    uint8_t infusion_lot_5;
    uint8_t infusion_lot_6;
    uint8_t infusion_lot_7;
    uint8_t infusion_lot_8;
    uint8_t infusion_lot_9;
    uint8_t infinite_lot_0;
    uint8_t infinite_lot_1;
    uint8_t infinite_lot_2;
    uint8_t infinite_lot_3;
    uint8_t infinite_lot_4;
    uint8_t infinite_lot_5;
    uint8_t infinite_lot_6;
    uint8_t infinite_lot_7;
    uint8_t infinite_lot_8;
    uint8_t infinite_lot_9;
    int32_t item_lot_0;
    int32_t item_lot_1;
    int32_t item_lot_2;
    int32_t item_lot_3;
    int32_t item_lot_4;
    int32_t item_lot_5;
    int32_t item_lot_6;
    int32_t item_lot_7;
    int32_t item_lot_8;
    int32_t item_lot_9;
    float_t chance_lot_0;
    float_t chance_lot_1;
    float_t chance_lot_2;
    float_t chance_lot_3;
    float_t chance_lot_4;
    float_t chance_lot_5;
    float_t chance_lot_6;
    float_t chance_lot_7;
    float_t chance_lot_8;
    float_t chance_lot_9;
} ITEM_LOT_PARAM2;

std::map<int32_t, StatBlock> get_weapon_requirements();
std::map<int32_t, int8_t> get_item_categories();
