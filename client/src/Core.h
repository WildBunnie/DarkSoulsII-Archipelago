#pragma once

#include "ArchipelagoInterface.h"

class ClientCore {
public:
    ClientCore();

    static VOID Start();
    static VOID InputCommand();
    static VOID Run();

    std::string pSlotName;
    std::string pPassword;
    std::string pSeed;

    static const int RUN_SLEEP = 2000;
};