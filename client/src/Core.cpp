#include "Core.h"
#include "hooks.h"
#include <spdlog/spdlog.h>

CArchipelago* acplg;
ClientCore* Core;
Hooks* GameHooks;

ClientCore::ClientCore() {
    Core = this;
    GameHooks = new Hooks();
}

VOID ClientCore::Start()
{
    Core = new ClientCore();

    spdlog::info("Archipelago client\n"
        "Type '/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}]' to connect to the room\n"
        "Type '/help' for more information\n"
        "-----------------------------------------------------");

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Core->InputCommand, NULL, NULL, NULL);
    
    while (true) {
        acplg->update();
        
        Core->HandleDeathLink();
        Core->HandleGiveItems();

        Sleep(1000/60); // run the loop 60 times per second
    }
}

VOID ClientCore::InputCommand()
{
    while (true) {
        std::string line;
        std::getline(std::cin, line);

        if (line == "/help") {
            spdlog::info("List of available commands : \n"
                "/help : Prints this help message.\n"
                "!help : Prints the help message related to Archipelago.\n"
                "/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}] : Connect to the specified server.\n"
                "/debug on|off : Prints additional debug info \n");
        }
        else if (line.find("/connect ") == 0) {
            std::string param = line.substr(9);
            int spaceIndex = param.find(" ");
            if (spaceIndex == std::string::npos) {
                spdlog::info("Missing parameter : Make sure to type '/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}]\n");
            }
            else {
                int passwordIndex = param.find("password:");
                std::string address = param.substr(0, spaceIndex);
                std::string slotName = param.substr(spaceIndex + 1, passwordIndex - spaceIndex - 2);
                std::string password = "";
                spdlog::info("{} - {}", address, slotName, "\n");
                Core->pSlotName = slotName;
                if (passwordIndex != std::string::npos)
                {
                    password = param.substr(passwordIndex + 9);
                }
                Core->pPassword = password;
                if (!acplg->Initialise(address)) {
                    spdlog::info("failed to connect to Archipelago\n");
                }
            }
        }
        else if (line.find("!") == 0) {
            acplg->say(line);
        }
        else {
            spdlog::info("Sorry did not understand that.\n");
        }
    }
}

VOID ClientCore::HandleDeathLink()
{
    if (GameHooks->isDeathLink && GameHooks->isPlayerInGame() && GameHooks->isPlayerDead()) {
        acplg->sendDeathLink();
    }
}

VOID ClientCore::HandleGiveItems()
{
    if (Core->itemsToGive.size() > 0 && GameHooks->isPlayerInGame() && Core->saveLoaded) {

        std::vector<int> items;

        for (const auto& networkItem : Core->itemsToGive) {
            if (networkItem.index < Core->lastReceivedIndex) {
                continue;
            }

            items.push_back(networkItem.item);

            if (items.size() == 8) {
                GameHooks->giveItems(items);
                Core->lastReceivedIndex += items.size();
                items.clear();
            }
        }
        if (!items.empty()) {
            GameHooks->giveItems(items);
            Core->lastReceivedIndex += items.size();
        }

        Core->itemsToGive.clear();
        Core->WriteSaveFile(); // this is probably not a good idea
    }
}

VOID ClientCore::WriteSaveFile() {
    try {
        std::filesystem::create_directory("archipelago");

        json j = {
            {"lastReceivedIndex", Core->lastReceivedIndex}
        };

        std::ofstream file("archipelago/" + std::string(Core->pSaveId) + "_" + std::string(Core->pSlotName) + ".json");

        file << j.dump(4);

        file.close();

        spdlog::debug("Successfully wrote save file");
    }
    catch (const std::exception& e) {
        spdlog::debug("Error writing save file");
    }
}

VOID ClientCore::ReadSaveFile() {
    try {
        std::ifstream file("archipelago/" + std::string(Core->pSaveId) + "_" + std::string(Core->pSlotName) + ".json");

        json j;
        file >> j;

        if (j.contains("lastReceivedIndex")) {
            Core->lastReceivedIndex = j["lastReceivedIndex"];
            spdlog::debug("Successfully read lastReceivedIndex: {}", Core->lastReceivedIndex);
            Core->saveLoaded = true;
        }

        file.close();
    }
    catch (const std::exception& e) {
        spdlog::debug("Error reading save file");
    }
}