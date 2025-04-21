#pragma once

#include <cstdint>
#include <vector>

#define NOMINMAX
#include <windows.h>

uintptr_t get_base_address();
uintptr_t resolve_pointer(uintptr_t base_address, uintptr_t address, std::vector<uintptr_t> offsets);
void patch_memory(uintptr_t address, const std::vector<uint8_t>& bytes);

template <typename T>
T read_value(uintptr_t address)
{
    T result;
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID*)address, &result, sizeof(T), NULL);
    return result;
}

template <typename T>
void write_value(uintptr_t address, T value)
{
    WriteProcessMemory(GetCurrentProcess(), (LPCVOID*)address, &value, sizeof(T), NULL);
}
