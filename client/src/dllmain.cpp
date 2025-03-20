#pragma comment(lib, "Crypt32.lib")

#include "Core.h"
#include "hooks.h"
#include "dinput8/dinput8.h"

extern ClientCore* Core;
extern Hooks* GameHooks;

DWORD WINAPI Init(LPVOID lpParam) {
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

    GameHooks->initHooks();

    Core->Start();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        LoadOriginalDll();
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Init, NULL, NULL, NULL);
    }
    else if(ul_reason_for_call == DLL_PROCESS_DETACH) {
        FreeOriginalDll();
    }
    return TRUE;
}