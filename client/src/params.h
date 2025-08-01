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
    uint8_t unk01[10];
    uint16_t row_amount;
    char header_text[52];
    ParamRow rows[];
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

std::map<int32_t, StatBlock> get_weapon_requirements();
std::map<int32_t, int8_t> get_item_categories();

void remove_weapon_requirements();
void remove_spell_requirements();
void remove_armor_requirements();
void remove_weight_from_params();

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

typedef struct {
    int32_t item_id;
    int32_t weapon_model_id;
    int32_t weapon_reinforce_id;
    int32_t weapon_action_category_id;
    int32_t weapon_type_id;
    int32_t Unk00;
    int16_t str_requirement;
    int16_t dex_requirement;
    int16_t int_requirement;
    int16_t fth_requirement;
    float_t weight;
    float_t recovery_animation_weight;
    float_t max_durability;
    int32_t base_repair_cost;
    int32_t Unk01;
    float_t stamina_consumption;
    float_t stamina_damage;
    int32_t weapon_stamina_cost_id;
    float_t Unk02;
    float_t Unk03;
    float_t Unk04;
    float_t Unk05;
    int32_t player_damage_id_backstab_small;
    int32_t player_damage_id_backstab_medium;
    int32_t player_damage_id_backstab_large;
    int32_t player_damage_id_guardbreak_small;
    int32_t player_damage_id_guardbreak_medium;
    int32_t player_damage_id_guardbreak_large;
    int32_t player_damage_id_riposte_small;
    int32_t player_damage_id_riposte_medium;
    int32_t player_damage_id_riposte_large;
    float_t parry_frames_duration;
    int32_t Unk06;
    int32_t Unk07;
    float_t damage_mult;
    float_t stamina_damage_mult;
    float_t durability_damage_mult;
    float_t guard_break_field;
    float_t status_effect_amount;
    float_t posture_damage_mult;
    float_t hitbox_radius;
    float_t hitbox_length;
    float_t hitback_radius;
    float_t hitback_length;
    int16_t damage_type_menu;
    int16_t poise_damage_menu;
    int16_t counter_damage_menu;
    int16_t casting_speed_menu;
    float_t poise_damage_pvp;
    float_t poise_damage_pve;
    float_t hyper_armor_poise_mult;
} WEAPON_PARAM;

typedef struct {
    int32_t spell_class;
    int8_t Unk00;
    int8_t spell_function;
    int16_t Unk01;
    uint16_t int_requirement;
    uint16_t fth_requirement;
    int32_t right_bullet_id;
    int32_t right_damage_id;
    float_t left_1h_startup_duration;
    float_t left_1h_unknown_duration;
    int32_t left_1h_anim_startup;
    int32_t left_1h_anim_cast;
    float_t left_1h_stamina_cost;
    float_t right_1h_startup_duration;
    float_t right_1h_unknown_duration;
    int32_t right_1h_anim_startup;
    int32_t right_1h_anim_cast;
    float_t right_1h_stamina_cost;
    float_t left_2h_startup_duration;
    float_t left_2h_unknown_duration;
    int32_t left_2h_anim_startup;
    int32_t left_2h_anim_cast;
    float_t left_2h_stamina_cost;
    float_t right_2h_startup_duration;
    float_t right_2h_unknown_duration;
    int32_t right_2h_anim_startup;
    int32_t right_2h_anim_cast;
    float_t right_2h_stamina_cost;
    float_t left_unkA_startup_duration;
    float_t left_unkA_unknown_duration;
    int32_t left_unkA_anim_startup;
    int32_t left_unkA_anim_cast;
    float_t left_unkA_stamina_cost;
    float_t right_unkA_startup_duration;
    float_t right_unkA_unknown_duration;
    int32_t right_unkA_anim_startup;
    int32_t right_unkA_anim_cast;
    float_t right_unkA_stamina_cost;
    float_t left_unkB_startup_duration;
    float_t left_unkB_unknown_duration;
    int32_t left_unkB_anim_startup;
    int32_t left_unkB_anim_cast;
    float_t left_unkB_stamina_cost;
    float_t right_unkB_startup_duration;
    float_t right_unkB_unknown_duration;
    int32_t right_unkB_anim_startup;
    int32_t right_unkB_anim_cast;
    float_t right_unkB_stamina_cost;
    float_t Unk29;
    uint8_t Unk30a;
    uint8_t is_area_of_effect_spell;
    uint8_t Unk30c;
    uint8_t Unk30d;
    int32_t Unk31;
    int32_t staff_sfx;
    uint8_t is_spawn_on_body;
    uint8_t staff_sfx_dmy_poly_id_1;
    uint8_t staff_sfx_dmy_poly_id_2;
    uint8_t Unk32;
    float_t Unk33;
    int32_t body_sfx;
    uint8_t is_spawn_on_weapon;
    uint8_t body_sfx_dmy_poly_id_1;
    uint8_t body_sfx_dmy_poly_id_2;
    uint8_t Unk34;
    float_t Unk35;
    float_t Unk36;
    float_t Unk37;
    float_t startup_speed;
    float_t cast_speed;
    float_t Unk38;
    float_t Unk39;
    uint8_t slots_used;
    uint8_t casts_tier_1;
    uint8_t casts_tier_2;
    uint8_t casts_tier_3;
    uint8_t casts_tier_4;
    uint8_t casts_tier_5;
    uint8_t casts_tier_6;
    uint8_t casts_tier_7;
    uint8_t casts_tier_8;
    uint8_t casts_tier_9;
    uint8_t casts_tier_10;
    uint8_t Unk40;
    int32_t spell_soul_consume_id;
    int32_t left_bullet_id;
    int32_t left_damage_id;
} SPELL_PARAM;

typedef struct {
    uint32_t armor_set_id;
    uint8_t Unk00;
    uint8_t slot_category;
    uint8_t Unk06;
    uint8_t Unk07;
    uint32_t model_id;
    uint8_t has_gendered_armor;
    uint8_t Unk08;
    int16_t Unk09;
    uint32_t equip_interfere_id;
    int32_t Unk10;
    uint32_t armor_reinforce_id;
    float_t physical_defence_scaling;
    float_t Unk11;
    float_t Unk12;
    float_t Unk13;
    uint16_t strength_requirement;
    uint16_t dexterity_requirement;
    uint16_t intelligence_requirement;
    uint16_t faith_requirement;
    float_t weight;
    float_t durability;
    uint32_t repair_cost;
    float_t poise;
    uint8_t control_parameters_blend_weight_1h;
    uint8_t control_parameters_blend_weight_2h;
    int16_t item_discovery;
    int32_t Unk15;
    int32_t Unk16;
    int32_t Unk17;
} ARMOR_PARAM;

typedef struct {
    float_t weight;
    float_t durability;
    int32_t repair_cost;
    int32_t item_discovery;
} RING_PARAM;