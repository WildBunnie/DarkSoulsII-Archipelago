#pragma once

#include <cstdint>

void patch_weapon_requirements(uintptr_t base_address);
void patch_spell_requirements(uintptr_t base_address);
void patch_infinite_torch(uintptr_t base_address);
void patch_unbreakable_chests(uintptr_t base_address);
void patch_disappearing_checks(uintptr_t base_address);
