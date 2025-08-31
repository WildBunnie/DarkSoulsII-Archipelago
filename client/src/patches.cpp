#include "patches.h"

#include "offsets.h"
#include "memory.h"

void patch_infinite_torch()
{
    patch_memory(get_base_address() + patches_offsets::infinite_torch, patches::infinite_torch);
}

