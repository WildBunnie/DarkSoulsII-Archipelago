#include "memory.h"

uintptr_t get_base_address() {
    HMODULE hModule = GetModuleHandleA("DarkSoulsII.exe");
    return (uintptr_t)hModule;
}

uintptr_t resolve_pointer(uintptr_t base_address, uintptr_t address, std::vector<uintptr_t> offsets)
{
    uintptr_t offset_null = NULL;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(base_address + address), &offset_null, sizeof(offset_null), 0);
    uintptr_t pointer_address = offset_null;
    for (size_t i = 0; i < offsets.size() - 1; i++)
    {
        ReadProcessMemory(GetCurrentProcess(), (LPVOID*)(pointer_address + offsets.at(i)), &pointer_address, sizeof(pointer_address), 0);
    }
    return pointer_address += offsets.at(offsets.size() - 1);
}

void patch_memory(uintptr_t address, const std::vector<uint8_t>& bytes) {
    DWORD old_protect;
    VirtualProtect(reinterpret_cast<void*>(address), bytes.size(), PAGE_EXECUTE_READWRITE, &old_protect);
    memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    VirtualProtect(reinterpret_cast<void*>(address), bytes.size(), old_protect, &old_protect);
}