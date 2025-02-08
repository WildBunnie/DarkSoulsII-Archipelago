#include "Core.h"

CArchipelago* acplg;

VOID ClientCore::Start()
{
    std::cout << "Archipelago client\n"
        "Type '/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}]' to connect to the room\n"
        "Type '/help' for more information\n"
        "-----------------------------------------------------\n";

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InputCommand, NULL, NULL, NULL);

    while (true) {
        acplg->update();
        Sleep(1000/60);
    }
}

VOID ClientCore::InputCommand()
{
    while (true) {
        std::string line;
        std::getline(std::cin, line);

        if (line == "/help") {
            printf("List of available commands : \n");
            printf("/help : Prints this help message.\n");
            printf("!help : Prints the help message related to Archipelago.\n");
            printf("/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}] : Connect to the specified server.\n");
            printf("/debug on|off : Prints additional debug info \n");
        }
        else if (line.find("/connect ") == 0) {
            std::string param = line.substr(9);
            int spaceIndex = param.find(" ");
            if (spaceIndex == std::string::npos) {
                std::cout << "Missing parameter : Make sure to type '/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}]\n'";
            }
            else {
                int passwordIndex = param.find("password:");
                std::string address = param.substr(0, spaceIndex);
                std::string slotName = param.substr(spaceIndex + 1, passwordIndex - spaceIndex - 2);
                std::string password = "";
                std::cout << address << " - " << slotName << "\n";
                //Core->pSlotName = slotName;
                if (passwordIndex != std::string::npos)
                {
                    password = param.substr(passwordIndex + 9);
                }
                //Core->pPassword = password;
                if (!acplg->Initialise(address)) {
                    std::cout << "failed to connect to Archipelago\n";
                }
            }
        }
        else if (line.find("!") == 0) {
            acplg->say(line);
        }
        else {
            std::cout << "Sorry did not understand that.";
        }
    }
}

VOID ClientCore::Run()
{
	return VOID();
}
