#pragma comment(lib, "Crypt32.lib")

#include <stdio.h>
#include <iostream>

#include "Core.h"
#include "hooks.h"

extern ClientCore* Core;

DWORD WINAPI Init(LPVOID lpParam);
void LoadOriginalDll();
HINSTANCE mHinstDLL = 0;
UINT_PTR mProcs[6] = { 0 };
LPCSTR mImportNames[] = { "DirectInput8Create", "DllCanUnloadNow", "DllGetClassObject", "DllRegisterServer", "DllUnregisterServer", "GetdfDIJoystick" };

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        // load the original library
        // and write the handle to dllHandle
        LoadOriginalDll();

        // get the address of the functions
        // from the original dll
        for (int i = 0; i < 6; i++) {
            mProcs[i] = (UINT_PTR)GetProcAddress(mHinstDLL, mImportNames[i]);
        }

        // start our stuff
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Init, NULL, NULL, NULL);
    }
    else if(ul_reason_for_call == DLL_PROCESS_DETACH) {
        FreeLibrary(mHinstDLL);
    }

    return TRUE;
}

DWORD WINAPI Init(LPVOID lpParam) {
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

    HMODULE hModule = GetModuleHandleA("DarkSoulsII.exe");
    DWORD baseAddress = (DWORD)hModule;

    initHooks();

    Core->Start();

    return 0;
}

// route our emulated functions
// to the original dll functions
extern "C" __declspec(naked) void __stdcall DirectInput8Create_wrapper() { __asm {jmp mProcs[0 * 4]} }
extern "C" __declspec(naked) void __stdcall DllCanUnloadNow_wrapper() { __asm {jmp mProcs[1 * 4]} }
extern "C" __declspec(naked) void __stdcall DllGetClassObject_wrapper() { __asm {jmp mProcs[2 * 4]} }
extern "C" __declspec(naked) void __stdcall DllRegisterServer_wrapper() { __asm {jmp mProcs[3 * 4]} }
extern "C" __declspec(naked) void __stdcall DllUnregisterServer_wrapper() { __asm {jmp mProcs[4 * 4]} }
extern "C" __declspec(naked) void __stdcall GetdfDIJoystick_wrapper() { __asm {jmp mProcs[5 * 4]} }

// Loads the original DLL from the default system directory
// Function originally written by Michael Koch
void LoadOriginalDll()
{
    char buffer[MAX_PATH];

    // Get path to system dir and to dinput8.dll
    GetSystemDirectoryA(buffer, MAX_PATH);

    // Append DLL name
    strcat_s(buffer, "\\dinput8.dll");

    // Try to load the system's dinput8.dll, if pointer empty
    if (!mHinstDLL) mHinstDLL = LoadLibraryA(buffer);

    // Debug
    if (!mHinstDLL)
    {
        OutputDebugStringA("PROXYDLL: Original dinput8.dll not loaded ERROR ****\r\n");
        ExitProcess(0); // Exit the hard way
    }
}