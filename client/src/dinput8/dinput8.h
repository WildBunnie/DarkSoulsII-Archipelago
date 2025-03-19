#pragma once

#include <windows.h>

HINSTANCE mHinstDLL = 0;
extern "C" UINT_PTR mProcs[6] = { 0 };

LPCSTR mImportNames[] = { "DirectInput8Create", "DllCanUnloadNow", "DllGetClassObject", "DllRegisterServer", "DllUnregisterServer", "GetdfDIJoystick" };

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

    for (int i = 0; i < 6; i++) {
        mProcs[i] = (UINT_PTR)GetProcAddress(mHinstDLL, mImportNames[i]);
    }
}

void FreeOriginalDll() {
    FreeLibrary(mHinstDLL);
}

extern "C" void DirectInput8Create_wrapper();
extern "C" void DllCanUnloadNow_wrapper();
extern "C" void DllGetClassObject_wrapper();
extern "C" void DllRegisterServer_wrapper();
extern "C" void DllUnregisterServer_wrapper();
extern "C" void GetdfDIJoystick_wrapper();