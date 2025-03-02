#pragma once

#include "ArchipelagoInterface.h"

enum GameVersion {
    SOTFS = 0,
    VANILLA = 1
};

class ClientCore {
public:
    ClientCore();

    static VOID Start();
    static VOID InputCommand();
    static VOID WriteSaveFile();
    static VOID ReadSaveFile();
    static VOID HandleDeathLink();
    static VOID HandleGiveItems();
    static VOID Panic(std::string message);

    std::string getSaveIdKey() {
        return "saveid_" + std::to_string(pTeamNumber) + "_" + pSlotName;
    }

    bool fatalError = false;

    std::string pSlotName;
    int pTeamNumber;
    std::string pPassword;
    std::string pSeed;
    std::string pSaveId;
    std::list<APClient::NetworkItem> itemsToGive;
    int lastReceivedIndex = 0;
    bool saveLoaded;
    bool diedByDeathLink = false;

#ifdef _M_IX86
    GameVersion gameVersion = VANILLA;
#elif defined(_M_X64)
    GameVersion gameVersion = SOTFS;
#endif

    static const int RUN_SLEEP = 2000;
};