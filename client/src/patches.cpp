#include "patches.h"

#include "offsets.h"
#include "memory.h"

void patch_weapon_requirements(uintptr_t base_address)
{
    // makes it so the function that checks requirements on onehand/twohand return without checking
    patch_memory(base_address + patches_offsets::no_weapon_req, patches::no_weapon_req_patch);
    // makes it so it doesn't load the values for the requirements to show in the menu
    // this is mostly to not show the "unable to use this item efficiently" message
    patch_memory(base_address + patches_offsets::menu_weapon_req, patches::menu_weapon_req_patch);
    // instead of multiplying the value of the requirements with 1.5x, we set the register to 0
    patch_memory(base_address + patches_offsets::no_power_stance_str_req, patches::no_power_stance_req);
    patch_memory(base_address + patches_offsets::no_power_stance_dex_req, patches::no_power_stance_req);
}

void patch_spell_requirements(uintptr_t base_address)
{
    patch_memory(base_address + patches_offsets::no_spell_cast_int_req, patches::no_spell_cast_req);
    patch_memory(base_address + patches_offsets::no_spell_cast_fth_req, patches::no_spell_cast_req);
    patch_memory(base_address + patches_offsets::no_spell_menu_int_req, patches::no_spell_menu_int_req);
    patch_memory(base_address + patches_offsets::no_spell_menu_fth_req, patches::no_spell_menu_fth_req);
}

void patch_infinite_torch(uintptr_t base_address)
{
    patch_memory(base_address + patches_offsets::infinite_torch, patches::infinite_torch);
}

void patch_unbreakable_chests(uintptr_t base_address)
{
    patch_memory(base_address + patches_offsets::unbreakable_chests, patches::unbreakable_chests);
}

void patch_disappearing_checks(uintptr_t base_address)
{
    // for some reason if the item doesn't fit in your inventory
    // the chest will be empty, so we just make sure it always spawns
    patch_memory(base_address + patches_offsets::disappearing_chest_items, patches::disappearing_chest_items);
}

void patch_no_equip_load(uintptr_t base_address)
{
    // sets equip load to 0 when you equip something
    patch_memory(base_address + patches_offsets::equip_load, patches::equip_load);
    // makes it so when you are hovering items to equip the equip load shows as 0
    patch_memory(base_address + patches_offsets::equip_load_menu, patches::equip_load_menu);
}

void patch_qol(uintptr_t base_address)
{
    // ported from https://github.com/r3sus/Resouls/tree/main/ds2s/mods
    // no logos
    patch_memory(base_address + patches_offsets::no_logos, { 0x01 });
    // no "press start button"
    patch_memory(base_address + patches_offsets::no_press_start, { 0x02 });
}

