#include "memory.h"
#include <windows.h>

uint32_t read_int(uintptr_t address) {
	uint32_t result;
	ReadProcessMemory(GetCurrentProcess(), (LPCVOID*)address, &result, sizeof(uint32_t), NULL);
	return result;
}