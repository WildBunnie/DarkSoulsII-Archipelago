#include "params.h"

#include "memory.h"
#include "offsets.h"

#include "spdlog/spdlog.h"

std::map<int32_t, StatBlock> get_weapon_requirements()
{
    uintptr_t param_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::weapon_param);
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + 0x44 - sizeof(uintptr_t)); // 0x3C for x64 and 0x40 for x86
    std::map<int32_t, StatBlock> result;

    for (int i = 0; i < 10000; i++) {
        int param_id = row_ptr[i].param_id;

        if (param_id == 12020000) break; // reached the end

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;

        StatBlock req;
        req.str = read_value<int16_t>(reward_ptr + 0x18);
        req.dex = read_value<int16_t>(reward_ptr + 0x1A);
        req.intl = read_value<int16_t>(reward_ptr + 0x1C);
        req.fth = read_value<int16_t>(reward_ptr + 0x1E);

        result[param_id] = req;
    }

    return result;
}

std::map<int32_t, int8_t> get_item_categories()
{
    uintptr_t param_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::item_param);
    ParamRow* row_ptr = reinterpret_cast<ParamRow*>(param_ptr + 0x44 - sizeof(uintptr_t)); // 0x3C for x64 and 0x40 for x86
    std::map<int32_t, int8_t> categories;

    for (int i = 0; i < 10000; ++i) {
        int param_id = row_ptr[i].param_id;
        if (i > 0 && param_id == 0) break;

        uintptr_t reward_ptr = param_ptr + row_ptr[i].reward_offset;

        categories[param_id] = read_value<int8_t>(reward_ptr + 0x4F);
    }

    return categories;
}

void remove_weapon_requirements()
{
    uintptr_t table_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::weapon_param);
    ParamTable* table = (ParamTable*)table_ptr;

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];
        WEAPON_PARAM* param = (WEAPON_PARAM*)(table_ptr + row->reward_offset);

        if (row->param_id < 1000000) continue; // ignore npc weapons

        param->str_requirement = 0;
        param->dex_requirement = 0;
        param->int_requirement = 0;
        param->fth_requirement = 0;
    }
}

void remove_spell_requirements()
{
    uintptr_t table_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::spell_param);
    ParamTable* table = (ParamTable*)table_ptr;

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];
        SPELL_PARAM* param = (SPELL_PARAM*)(table_ptr + row->reward_offset);

        param->int_requirement = 0;
        param->fth_requirement = 0;
    }
}

void remove_armor_requirements()
{
    uintptr_t table_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::armor_param);
    ParamTable* table = (ParamTable*)table_ptr;

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];
        ARMOR_PARAM* param = (ARMOR_PARAM*)(table_ptr + row->reward_offset);

        param->strength_requirement = 0;
        param->dexterity_requirement = 0;
        param->intelligence_requirement = 0;
        param->faith_requirement = 0;
    }
}

void remove_weight_from_params()
{
    uintptr_t table_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::weapon_param);
    ParamTable* table = (ParamTable*)table_ptr;

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];
        WEAPON_PARAM* param = (WEAPON_PARAM*)(table_ptr + row->reward_offset);

        if (row->param_id < 1000000) continue; // ignore npc weapons

        param->weight = 0.0f;
    }

    table_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::ring_param);
    table = (ParamTable*)table_ptr;

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];
        RING_PARAM* param = (RING_PARAM*)(table_ptr + row->reward_offset);

        param->weight = 0.0f;
    }

    table_ptr = resolve_pointer(get_base_address(), pointer_offsets::base_a, param_offsets::armor_param);
    table = (ParamTable*)table_ptr;

    for (int i = 0; i < table->row_amount; ++i) {
        ParamRow* row = &table->rows[i];
        ARMOR_PARAM* param = (ARMOR_PARAM*)(table_ptr + row->reward_offset);

        param->weight = 0.0f;
    }
}