#pragma once

#include "ArchipelagoInterface.h"

class ClientCore {
public:
    ClientCore();

    static VOID Start();
    static VOID InputCommand();
    static VOID Run();
    static VOID WriteSaveFile();
    static VOID ReadSaveFile();
    static VOID HandleDeathLink();
    static VOID HandleGiveItems();

    std::string getSaveIdKey() {
        return "saveid_" + std::to_string(pTeamNumber) + "_" + pSlotName;
    }

    std::string pSlotName;
    int pTeamNumber;
    std::string pPassword;
    std::string pSeed;
    std::string pSaveId;
    std::list<APClient::NetworkItem> itemsToGive;
    int lastReceivedIndex = 0;
    bool saveLoaded;
    bool diedByDeathLink = false;

    static const int RUN_SLEEP = 2000;
};