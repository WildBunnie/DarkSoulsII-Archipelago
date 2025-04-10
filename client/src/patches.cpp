#include "patches.h"

#include "offsets.h"
#include "memory.h"

void patch_weapon_requirements(uintptr_t base_address) {
    // makes it so the function that checks requirements on onehand/twohand return without checking
    patch_memory(base_address + PatchesOffsets::noWeaponReqPatchOffset, Patches::noWeaponReqPatch);
    // makes it so it doesnt load the values for the requirements to show in the menu
    // this is mostly to not show the "unable to use this item efficiently" message
    patch_memory(base_address + PatchesOffsets::menuWeaponReqPatchOffset, Patches::menuWeaponReqPatch);
    // instead of multiplying the value of the requirements with 1.5x, we set the register to 0
    patch_memory(base_address + PatchesOffsets::noPowerStanceStrReq, Patches::noPowerStanceReq);
    patch_memory(base_address + PatchesOffsets::noPowerStanceDexReq, Patches::noPowerStanceReq);
}

void patch_spell_requirements(uintptr_t base_address) {
    patch_memory(base_address + PatchesOffsets::noSpellCastIntReq, Patches::noSpellCastReq);
    patch_memory(base_address + PatchesOffsets::noSpellCastFthReq, Patches::noSpellCastReq);
    patch_memory(base_address + PatchesOffsets::noSpellMenuIntReq, Patches::noSpellMenuIntReq);
    patch_memory(base_address + PatchesOffsets::noSpellMenuFthReq, Patches::noSpellMenuFthReq);
}

void patch_infinite_torch(uintptr_t base_address) {
    patch_memory(base_address + PatchesOffsets::infiniteTorchOffset, Patches::infiniteTorch);
}

void patch_unbreakable_chests(uintptr_t base_address) {
    patch_memory(base_address + PatchesOffsets::UnbreakableChests, Patches::UnbreakableChests);
}

void patch_disappearing_checks(uintptr_t base_address) {
    // for some reason if the item doesnt fit in your inventory
    // the chest will be empty, so we just make sure it always spawns
    patch_memory(base_address + PatchesOffsets::DissapearingChestItems, Patches::DissapearingChestItems);
}
