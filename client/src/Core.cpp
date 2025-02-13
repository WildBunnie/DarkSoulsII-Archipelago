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
        "-----------------------------------------------------\n");

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Core->InputCommand, NULL, NULL, NULL);

    
    while (true) {
        acplg->update();
        
        if (GameHooks->isPlayerInGame() && GameHooks->isPlayerDead()) {
            acplg->sendDeathLink();
        }

        if (Core->itemsToGive.size() > 0 && GameHooks->isPlayerInGame()) {
            size_t totalItems = Core->itemsToGive.size();
            size_t batchSize = 8;
            for (int i = 0; i < totalItems; i += batchSize) {
                std::vector<int> batch(Core->itemsToGive.begin() + i,
                                       Core->itemsToGive.begin() + std::min(i + batchSize, totalItems));

                GameHooks->giveItems(batch);

                Core->itemsToGive.erase(Core->itemsToGive.begin() + i,
                                        Core->itemsToGive.begin() + std::min(i + batchSize, totalItems));

                totalItems = Core->itemsToGive.size();
            }
        }

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

VOID ClientCore::Run()
{
	return VOID();
}
