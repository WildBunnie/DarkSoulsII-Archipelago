#include "patches.h"

#include "offsets.h"
#include "memory.h"

void patch_infinite_torch(uintptr_t base_address)
{
    patch_memory(base_address + patches_offsets::infinite_torch, patches::infinite_torch);
}

