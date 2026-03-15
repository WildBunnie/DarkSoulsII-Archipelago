#pragma once
#include <stdint.h>
#include <windows.h>

#ifdef _M_X64
    #define DS2_64 1
    #define DS2_32 0
#elif defined(_M_IX86)
    #define DS2_64 0
    #define DS2_32 1
#endif

#if DS2_64
    #define DS2_OFFSET(x64, x86) (x64)
#else
    #define DS2_OFFSET(x64, x86) (x86)
#endif

/* =========================================================
   Params
   ========================================================= */

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
    float chance_lot_0;
    float chance_lot_1;
    float chance_lot_2;
    float chance_lot_3;
    float chance_lot_4;
    float chance_lot_5;
    float chance_lot_6;
    float chance_lot_7;
    float chance_lot_8;
    float chance_lot_9;
} DS2ItemLotParam2;

typedef struct {
    int32_t item_id;
    int32_t Unk00;
    int32_t enable_flag;
    int32_t disable_flag;
    int32_t material_id;
    int32_t duplicate_item_id;
    int32_t Unk01;
    float price_rate;
    int32_t quantity;
} DS2ShopLineupParam;

typedef struct {
    int32_t icon_id;
    int32_t speffect_id;
    int32_t sfx_on_use_1;
    int32_t sfx_on_use_2;
    int32_t sfx_on_use_3;
    int32_t weapon_id;
    int32_t armor_id;
    int32_t ammo_id;
    int32_t ring_id;
    int32_t spell_id;
    int32_t gesture_id;
    int32_t sort_id;
    int32_t base_price;
    int32_t sell_price;
    float   animation_speed;
    int32_t Unk03;
    int32_t item_type_id;
    int32_t item_usage_id;
    uint8_t Unk04;
    uint8_t Unk05;
    uint16_t max_held_count;
    uint8_t speffect_context;
    uint8_t item_sp_effect_type;
    uint8_t Unk06;
    uint8_t item_category_type;
    uint8_t item_speffect_application_type;
    uint8_t item_additional_type;
    uint8_t ng_behavior_type;
    uint8_t Unk07;
} DS2ItemParam;

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
    float weight;
    float recovery_animation_weight;
    float max_durability;
    int32_t base_repair_cost;
    int32_t Unk01;
    float stamina_consumption;
    float stamina_damage;
    int32_t weapon_stamina_cost_id;
    float Unk02;
    float Unk03;
    float Unk04;
    float Unk05;
    int32_t player_damage_id_backstab_small;
    int32_t player_damage_id_backstab_medium;
    int32_t player_damage_id_backstab_large;
    int32_t player_damage_id_guardbreak_small;
    int32_t player_damage_id_guardbreak_medium;
    int32_t player_damage_id_guardbreak_large;
    int32_t player_damage_id_riposte_small;
    int32_t player_damage_id_riposte_medium;
    int32_t player_damage_id_riposte_large;
    float parry_frames_duration;
    int32_t Unk06;
    int32_t Unk07;
    float damage_mult;
    float stamina_damage_mult;
    float durability_damage_mult;
    float guard_break_field;
    float status_effect_amount;
    float posture_damage_mult;
    float hitbox_radius;
    float hitbox_length;
    float hitback_radius;
    float hitback_length;
    int16_t damage_type_menu;
    int16_t poise_damage_menu;
    int16_t counter_damage_menu;
    int16_t casting_speed_menu;
    float poise_damage_pvp;
    float poise_damage_pve;
    float hyper_armor_poise_mult;
} DS2WeaponParam;

typedef struct {
    int32_t spell_class;
    int8_t Unk00;
    int8_t spell_function;
    int16_t Unk01;
    uint16_t int_requirement;
    uint16_t fth_requirement;
    int32_t right_bullet_id;
    int32_t right_damage_id;
    float left_1h_startup_duration;
    float left_1h_unknown_duration;
    int32_t left_1h_anim_startup;
    int32_t left_1h_anim_cast;
    float left_1h_stamina_cost;
    float right_1h_startup_duration;
    float right_1h_unknown_duration;
    int32_t right_1h_anim_startup;
    int32_t right_1h_anim_cast;
    float right_1h_stamina_cost;
    float left_2h_startup_duration;
    float left_2h_unknown_duration;
    int32_t left_2h_anim_startup;
    int32_t left_2h_anim_cast;
    float left_2h_stamina_cost;
    float right_2h_startup_duration;
    float right_2h_unknown_duration;
    int32_t right_2h_anim_startup;
    int32_t right_2h_anim_cast;
    float right_2h_stamina_cost;
    float left_unkA_startup_duration;
    float left_unkA_unknown_duration;
    int32_t left_unkA_anim_startup;
    int32_t left_unkA_anim_cast;
    float left_unkA_stamina_cost;
    float right_unkA_startup_duration;
    float right_unkA_unknown_duration;
    int32_t right_unkA_anim_startup;
    int32_t right_unkA_anim_cast;
    float right_unkA_stamina_cost;
    float left_unkB_startup_duration;
    float left_unkB_unknown_duration;
    int32_t left_unkB_anim_startup;
    int32_t left_unkB_anim_cast;
    float left_unkB_stamina_cost;
    float right_unkB_startup_duration;
    float right_unkB_unknown_duration;
    int32_t right_unkB_anim_startup;
    int32_t right_unkB_anim_cast;
    float right_unkB_stamina_cost;
    float Unk29;
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
    float Unk33;
    int32_t body_sfx;
    uint8_t is_spawn_on_weapon;
    uint8_t body_sfx_dmy_poly_id_1;
    uint8_t body_sfx_dmy_poly_id_2;
    uint8_t Unk34;
    float Unk35;
    float Unk36;
    float Unk37;
    float startup_speed;
    float cast_speed;
    float Unk38;
    float Unk39;
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
} DS2SpellParam;

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
    float physical_defence_scaling;
    float Unk11;
    float Unk12;
    float Unk13;
    uint16_t strength_requirement;
    uint16_t dexterity_requirement;
    uint16_t intelligence_requirement;
    uint16_t faith_requirement;
    float weight;
    float durability;
    uint32_t repair_cost;
    float poise;
    uint8_t control_parameters_blend_weight_1h;
    uint8_t control_parameters_blend_weight_2h;
    int16_t item_discovery;
    int32_t Unk15;
    int32_t Unk16;
    int32_t Unk17;
} DS2ArmorParam;

typedef struct {
    float weight;
    float durability;
    int32_t repair_cost;
    int32_t item_discovery;
} DS2RingParam;

/* =========================================================
   Enums
   ========================================================= */

typedef enum {
    DS2_ItemLotParam2_Chr      = 0,
    DS2_ItemLotParam2_Other    = 1,
    DS2_ItemLotParam2_SvrEvent = 2,
} DS2ItemLotCategory;

typedef enum {
    DS2_GAMESTATE_MENU   = 10,
    DS2_GAMESTATE_INGAME = 30
} DS2GameState;

typedef enum {
    DS2_FMG_ITEM_NAMES     = 8,
    DS2_FMG_ITEM_SUMMARIES = 9
} DS2FMGFileID;

/* =========================================================
   Game Structures
   ========================================================= */

typedef struct {
    uint32_t unk01[2];
    uint32_t item_lot_id;
    uint16_t unk02;
    uint16_t unk03;
    uint8_t unk04;
    uint8_t unk05;
    uint8_t picked_up;
    uint8_t item_lot_category; // DS2ItemLotCategory
} DS2MapItemPackEntityData;

typedef struct {
    uint32_t item_lot_id;
    float unk01;
} DS2ItemLotReward;

typedef struct {
    uintptr_t unk01[2];
    uint32_t shop_lineup_id;
    uint32_t item_id;
    // additional fields omitted
} DS2BoughtShopItem;

typedef struct {
    uint32_t unk01;
    uint32_t item_id;
    float durability;
    uint16_t amount;
    uint8_t upgrade_level;
    uint8_t infusion;
} DS2ItemStruct;

typedef struct {
    DS2ItemStruct items[8];
    uint8_t item_count;
    uint8_t unk01;
    uint8_t unk02;
    uint8_t unk03;
} DS2ItemBagContent;

#if DS2_64
typedef struct {
    uint8_t padding1[4];
    int32_t param_id;
    uint8_t padding2[4];
    int32_t reward_offset;
    uint8_t padding3[4];
    int32_t unk01;
} DS2ParamRow;
#elif DS2_32
typedef struct {
    int32_t param_id;
    int32_t reward_offset;
    int32_t unk01;
} DS2ParamRow;
#endif

typedef struct {
    uint8_t unk01[10];
    uint16_t row_amount;
    char header_text[DS2_OFFSET(48, 52)];
    DS2ParamRow rows[];
} DS2ParamTable;

/* =========================================================
   Managers
   ========================================================= */

typedef struct {
    uint8_t unk01;
} DS2EventFlagManager;

typedef struct {
    uint8_t unk01[DS2_OFFSET(0x20, 0x10)];
    DS2EventFlagManager* event_flag_manager; // 0x20 / 0x10
} DS2EventManager;

typedef struct {
    uint8_t unk01;
} DS2GameDataManager;

typedef struct {
    uint8_t unk01;
} DS2DLBackAllocator;

/* =========================================================
   DS2GameManagerImp
   ========================================================= */

typedef struct {
    uint8_t unk01[DS2_OFFSET(0x70, 0x44)];
    DS2EventManager* event_manager;           // 0x70 / 0x44

    uint8_t unk02[DS2_OFFSET(0x30, 0x18)];
    DS2GameDataManager* game_data_manager;    // 0xA8 / 0x60

    uint8_t unk04[DS2_OFFSET(0x2230, 0xC60)];
    DS2DLBackAllocator* dl_back_allocator;    // 0x22E0 / 0xCC4

    uint8_t unk05[DS2_OFFSET(0x1C4, 0x124)];
    int game_state; // DS2GameState           // 0x24AC / 0xDEC
} DS2GameManagerImp;

DS2GameManagerImp* ds2_game_manager_imp = 0;

/* =========================================================
   Static Addresses
   ========================================================= */

#define DS2_SINGLETON_GameManagerImp              DS2_OFFSET(0x16148f0, 0x1150414)

#define DS2_FUNCTION_SET_MAP_ENTITY_PICKED_UP     DS2_OFFSET(0x1DE6C0, 0x257060)
#define DS2_FUNCTION_GIVE_ITEMS_ON_REWARD         DS2_OFFSET(0x199CC0, 0x21D3C0)
#define DS2_FUNCTION_GIVE_ITEMS_ON_SHOP_PURCHASE  DS2_OFFSET(0x1A76A0, 0x22B340)

#define DS2_FUNCTION_ADD_ITEM_TO_INVENTORY        DS2_OFFSET(0x1AA810, 0x229C00)
#define DS2_FUNCTION_REMOVE_ITEM_FROM_INVENTORY   DS2_OFFSET(0x1AF1E0, 0x233ED0)

#define DS2_FUNCTION_ADD_ITEMS_TO_INVENTORY       DS2_OFFSET(0x1A7470, 0x22AD20)
#define DS2_FUNCTION_CAN_INVENTORY_FIT_ITEMS      DS2_OFFSET(0x1A60B0, 0x22AB50)
#define DS2_FUNCTION_CREATE_POPUP_STRUCTURE       DS2_OFFSET(0x5D950, 0x11F430)
#define DS2_FUNCTION_SHOW_ITEM_POPUP              DS2_OFFSET(0x501080, 0x4FA9B0)

#define DS2_FUNCTION_WRITE_EVENT_FLAG             DS2_OFFSET(0x4750B0, 0x47FAE0)
#define DS2_FUNCTION_LOAD_REGION_EVENT_FLAGS      DS2_OFFSET(0x4745C0, 0x47FFE0)
#define DS2_FUNCTION_GET_FMG_ENTRY                DS2_OFFSET(0x503620, 0x4FF6E0)
#define DS2_FUNCTION_GET_HOVERING_ITEM_INFO       DS2_OFFSET(0x38170,  0x105A40)

#define DS2_FUNCTION_SAVE_GAME                    DS2_OFFSET(0x2E78C0, 0x31B3F0)

/* =========================================================
   Pointer Chains
   ========================================================= */

static uintptr_t item_lot_param2_other_chain[] = {
    DS2_SINGLETON_GameManagerImp,
    DS2_OFFSET(0xA8, 0x60),
    DS2_OFFSET(0x60, 0x30),
    DS2_OFFSET(0xD8, 0x94),
    0x0
};
uintptr_t DS2_PARAM_ITEM_LOT_PARAM2_OTHER = 0;

static uintptr_t item_lot_param2_chr_chain[] = {
    DS2_SINGLETON_GameManagerImp,
    DS2_OFFSET(0xA8, 0x60),
    DS2_OFFSET(0x50, 0x28),
    DS2_OFFSET(0xD8, 0x94),
    0x0
};
uintptr_t DS2_PARAM_ITEM_LOT_PARAM2_CHR = 0;

static uintptr_t shop_lineup_param_chain[] = {
    DS2_SINGLETON_GameManagerImp,
    DS2_OFFSET(0xA8, 0x60),
    DS2_OFFSET(0xB0, 0x58),
    DS2_OFFSET(0xD8, 0x94),
    0x0
};
uintptr_t DS2_PARAM_SHOP_LINEUP_PARAM = 0;

static uintptr_t item_param_chain[] = {
    DS2_SINGLETON_GameManagerImp,
    DS2_OFFSET(0xA8, 0x60),
    DS2_OFFSET(0x20, 0x10),
    DS2_OFFSET(0xD8, 0x94),
    0x0
};
uintptr_t DS2_PARAM_ITEM_PARAM = 0;

static uintptr_t weapon_param_chain[] = {
    DS2_SINGLETON_GameManagerImp,
    DS2_OFFSET(0x18, 0x18),
    DS2_OFFSET(0x420, 0x210),
    DS2_OFFSET(0xD8, 0x94),
    0x0
};
uintptr_t DS2_PARAM_WEAPON_PARAM = 0;

static uintptr_t spell_param_chain[] = {
    DS2_SINGLETON_GameManagerImp,
    DS2_OFFSET(0x18, 0x18),
    DS2_OFFSET(0x540, 0x2A0),
    DS2_OFFSET(0xD8, 0x94),
    0x0
};
uintptr_t DS2_PARAM_SPELL_PARAM = 0;

static uintptr_t armor_param_chain[] = {
    DS2_SINGLETON_GameManagerImp,
    DS2_OFFSET(0x18, 0x18),
    DS2_OFFSET(0x4A0, 0x250),
    DS2_OFFSET(0xD8, 0x94),
    0x0
};
uintptr_t DS2_PARAM_ARMOR_PARAM = 0;

static uintptr_t ring_param_chain[] = {
    DS2_SINGLETON_GameManagerImp,
    DS2_OFFSET(0x18, 0x18),
    DS2_OFFSET(0x530, 0x298),
    DS2_OFFSET(0xD8, 0x94),
    0x0
};
uintptr_t DS2_PARAM_RING_PARAM = 0;

/* =========================================================
   Function Typedefs
   ========================================================= */

typedef void (__thiscall *ds2_set_map_entity_picked_up_t)(
    DS2MapItemPackEntityData* param_1,
    DS2MapItemPackEntityData* param_2
);

typedef uint8_t (__thiscall *ds2_give_items_on_reward_t)(
    uintptr_t param_1, // EventWindowManager
    DS2ItemLotReward* reward,
    int32_t window_display_type, // 1 - default | 2 - inventory full | 3 - no accept button | 4+ - indefinite bugged window
    int8_t param_4,
    DS2ItemLotCategory item_lot_category
);

typedef uintptr_t (__thiscall *ds2_give_items_on_shop_purchase_t)(
    uintptr_t param_1,
    DS2BoughtShopItem* bought,
    uint16_t amount
);

typedef void (__thiscall *ds2_add_item_to_inventory_t)(
    uintptr_t param_1, // ItemInventory2BagList
    DS2ItemStruct* item
);
ds2_add_item_to_inventory_t ds2_add_item_to_inventory = 0;

typedef uint32_t (__thiscall *ds2_remove_item_from_inventory_t)(
    uintptr_t param_1, // ItemInventory2NormalList
    uintptr_t param_2, // ItemInventory2BagList
    uintptr_t item,
    uint32_t quantity
);
ds2_remove_item_from_inventory_t ds2_remove_item_from_inventory = 0;

typedef uint8_t (__thiscall *ds2_add_items_to_inventory_t)(
    uintptr_t param_1, // ItemInventory2BagList
    DS2ItemBagContent* item_bag,
    uint32_t amount,
    int32_t skip_save
);
ds2_add_items_to_inventory_t ds2_add_items_to_inventory = 0;

typedef uint8_t (__thiscall *ds2_can_inventory_fit_items_t)(
    uintptr_t param_1, // ItemInventory2BagList
    DS2ItemBagContent* item_bag,
    uint32_t amount
);
ds2_can_inventory_fit_items_t ds2_can_inventory_fit_items = 0;

typedef void (__cdecl *ds2_create_popup_structure_t)(
    uintptr_t out_popup_struct,
    DS2ItemBagContent* item_bag,
    uint32_t num_items,
    int32_t display_type // 1 - default | 2 - inventory full | 3 - no accept button | 4+ - indefinite bugged window
);
ds2_create_popup_structure_t ds2_create_popup_structure = 0;

typedef void (__thiscall *ds2_show_item_popup_t)(
    DS2DLBackAllocator* allocator,
    uintptr_t popup_struct
);
ds2_show_item_popup_t ds2_show_item_popup = 0;

typedef uint8_t (__thiscall *ds2_write_event_flag_t)(
    DS2EventFlagManager* manager,
    uint32_t flag_id,
    uint8_t state
);
ds2_write_event_flag_t ds2_write_event_flag = 0;

typedef const wchar_t* (__cdecl *ds2_get_fmg_entry_t)(
    DS2FMGFileID file_id,
    int32_t entry_id
);
ds2_get_fmg_entry_t ds2_get_fmg_entry = 0;

typedef const uintptr_t (__thiscall *ds2_get_hovering_item_info_t)(
    uintptr_t param_1,
    uintptr_t param_2
);
ds2_get_hovering_item_info_t ds2_get_hovering_item_info = 0;

typedef const void (__thiscall *ds2_load_region_event_flags_t)(
    DS2EventFlagManager* manager,
    uint32_t region_id // NOTE not sure about this
);
ds2_load_region_event_flags_t ds2_load_region_event_flags = 0;

typedef const uint8_t (__thiscall *ds2_save_game_t)(
    uintptr_t param_1, // SaveLoadSystem
    uint8_t param_2,
    uint8_t param_3
);
ds2_save_game_t ds2_save_game = 0;

/* =========================================================
   Public API
   ========================================================= */

// This is not a struct from the game
// this is here to make patching params easier
typedef struct {
    uintptr_t table_ptr;
    DS2ParamRow* row;
    void* data;
} ParamRowData;

typedef void (*param_patch_fn)(
    ParamRowData* row_data,
    void* context
);

int libds2_init();
int libds2_give_items(DS2ItemBagContent items);
int libds2_is_player_ingame();
int libds2_set_event_flag(uint32_t flag_id, uint8_t state);
int libds2_is_item_popup_open();
int libds2_patch_param_table(uintptr_t table_ptr, param_patch_fn fn, void* context);

/* =========================================================
   Implementation
   ========================================================= */

#ifdef LIBDS2_IMPLEMENTATION

static inline uintptr_t resolve_pointer_chain(uintptr_t base, uintptr_t offsets[], int offset_count)
{
    uintptr_t addr = base;

    for (int i = 0; i < offset_count - 1; i++)
    {
        addr += offsets[i];
        addr = *(uintptr_t*)addr;
        if (!addr) return 0;
    }

    addr += offsets[offset_count - 1];
    return addr;
}

static uintptr_t resolve_pointer_chain_retry(uintptr_t base, uintptr_t* offsets, int offset_count)
{
    for (int i = 0; i < 300; ++i) {
        uintptr_t addr = resolve_pointer_chain(base, offsets, offset_count);
        if (addr)
            return addr;

        Sleep(1000/60);
    }

    return 0;
}

// TODO check for errors
int libds2_init()
{
    uintptr_t base_address = (uintptr_t)GetModuleHandle("DarkSoulsII.exe");
    uintptr_t game_manager_imp_addr = base_address + DS2_SINGLETON_GameManagerImp;

    for (int attempts = 0; attempts < 10000; ++attempts) {
        ds2_game_manager_imp = *(DS2GameManagerImp**)game_manager_imp_addr;
        if (ds2_game_manager_imp) break;
        Sleep(1000 / 60);
    }
    if (!ds2_game_manager_imp) return 0;

    // Initialize stuff
    ds2_add_items_to_inventory = (ds2_add_items_to_inventory_t)(base_address + DS2_FUNCTION_ADD_ITEMS_TO_INVENTORY);
    ds2_create_popup_structure = (ds2_create_popup_structure_t)(base_address + DS2_FUNCTION_CREATE_POPUP_STRUCTURE);
    ds2_show_item_popup = (ds2_show_item_popup_t)(base_address + DS2_FUNCTION_SHOW_ITEM_POPUP);
    ds2_can_inventory_fit_items = (ds2_can_inventory_fit_items_t)(base_address + DS2_FUNCTION_CAN_INVENTORY_FIT_ITEMS);

    ds2_write_event_flag = (ds2_write_event_flag_t)(base_address + DS2_FUNCTION_WRITE_EVENT_FLAG);

    DS2_PARAM_ITEM_LOT_PARAM2_OTHER = resolve_pointer_chain_retry(
        base_address, item_lot_param2_other_chain,
        sizeof(item_lot_param2_other_chain) / sizeof(item_lot_param2_other_chain[0])
    );

    DS2_PARAM_ITEM_LOT_PARAM2_CHR = resolve_pointer_chain_retry(
        base_address, item_lot_param2_chr_chain,
        sizeof(item_lot_param2_chr_chain) / sizeof(item_lot_param2_chr_chain[0])
    );

    DS2_PARAM_SHOP_LINEUP_PARAM = resolve_pointer_chain_retry(
        base_address, shop_lineup_param_chain,
        sizeof(shop_lineup_param_chain) / sizeof(shop_lineup_param_chain[0])
    );

    DS2_PARAM_ITEM_PARAM = resolve_pointer_chain_retry(
        base_address, item_param_chain,
        sizeof(item_param_chain) / sizeof(item_param_chain[0])
    );

    DS2_PARAM_WEAPON_PARAM = resolve_pointer_chain_retry(
        base_address, weapon_param_chain,
        sizeof(weapon_param_chain) / sizeof(weapon_param_chain[0])
    );

    DS2_PARAM_SPELL_PARAM = resolve_pointer_chain_retry(
        base_address, spell_param_chain,
        sizeof(spell_param_chain) / sizeof(spell_param_chain[0])
    );

    DS2_PARAM_ARMOR_PARAM = resolve_pointer_chain_retry(
        base_address, armor_param_chain,
        sizeof(armor_param_chain) / sizeof(armor_param_chain[0])
    );

    DS2_PARAM_RING_PARAM = resolve_pointer_chain_retry(
        base_address, ring_param_chain,
        sizeof(ring_param_chain) / sizeof(ring_param_chain[0])
    );

    return 1;
}

int libds2_give_items(DS2ItemBagContent item_bag)
{
    uint8_t display_struct[0x200];

    uintptr_t game_data_manager = (uintptr_t)ds2_game_manager_imp->game_data_manager;
    uintptr_t unk = *(uintptr_t*)(game_data_manager + DS2_OFFSET(0x10, 0x8));
    uintptr_t item_inventory_2_bag_list = *(uintptr_t*)(unk + DS2_OFFSET(0x10, 0x8));

    int display_mode;
    if (!ds2_can_inventory_fit_items(item_inventory_2_bag_list, &item_bag, item_bag.item_count)) {
        DS2ItemBagContent new_bag = {0};
        for (int i = 0; i < item_bag.item_count; ++i) {
            DS2ItemBagContent test_bag = {0};
            test_bag.item_count = 1;
            test_bag.items[0] = item_bag.items[i];
            if (ds2_can_inventory_fit_items(item_inventory_2_bag_list, &test_bag, 1)) {
                new_bag.items[new_bag.item_count++] = item_bag.items[i];
            }
        }
        ds2_add_items_to_inventory(item_inventory_2_bag_list, &new_bag, new_bag.item_count, 0);
        display_mode = 2; // inventory full
    }
    else {
        ds2_add_items_to_inventory(item_inventory_2_bag_list, &item_bag, item_bag.item_count, 0);
        display_mode = 1; // default
    }

    ds2_create_popup_structure((uintptr_t)display_struct, &item_bag, item_bag.item_count, display_mode);
    ds2_show_item_popup(ds2_game_manager_imp->dl_back_allocator, (uintptr_t)display_struct);

    return 1;
}

int libds2_get_player_state()
{
    return ds2_game_manager_imp->game_state;
}

int libds2_set_event_flag(uint32_t flag_id, uint8_t state)
{
    return ds2_write_event_flag(
        ds2_game_manager_imp->event_manager->event_flag_manager,
        flag_id,
        state
    );
}

int libds2_is_item_popup_open()
{
    uintptr_t unk01 = *(uintptr_t*)(ds2_game_manager_imp->dl_back_allocator + DS2_OFFSET(0xD8, 0x6C));
    uintptr_t unk02 = *(uintptr_t*)(unk01 + DS2_OFFSET(0x390, 0x234));
    int32_t flag = *(int32_t*)(unk02 + DS2_OFFSET(0x24, 0x18));
    return flag != 0;
}

int libds2_patch_param_table(uintptr_t table_ptr, param_patch_fn fn, void* context)
{
    if (!table_ptr || !fn)
        return 0;

    DS2ParamTable* table = (DS2ParamTable*)table_ptr;

    for (int i = 0; i < table->row_amount; ++i) {
        DS2ParamRow* row = &table->rows[i];

        ParamRowData row_data = {0};
        row_data.table_ptr = table_ptr;
        row_data.row = row;
        row_data.data = (void*)(table_ptr + row->reward_offset);

        fn(&row_data, context);
    }

    return 1;
}

#endif // LIBDS2_IMPLEMENTATION