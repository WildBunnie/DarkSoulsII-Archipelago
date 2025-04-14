#pragma comment(lib, "Crypt32.lib")

#include "dinput8/dinput8.h"
#include "archipelago.h"
#include "hooks.h"
#include "game_functions.h"
#include "offsets.h"
#include "ds2.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>

void setup_logging()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("archipelago/archipelago.log", false);

    console_sink->set_level(spdlog::level::debug);
    file_sink->set_level(spdlog::level::debug);

#ifdef NDEBUG
    console_sink->set_level(spdlog::level::info);
#endif

    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    auto logger = std::make_shared<spdlog::logger>("DS2Archipelago", spdlog::sinks_init_list{ console_sink, file_sink });
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
}

void handle_input()
{
    while (true) {
        std::string line;
        std::getline(std::cin, line);

        if (line == "/help") {
            spdlog::info("List of available commands : \n"
                "/help : Prints this help message.\n"
                "!help : Prints the help message related to Archipelago.\n"
                "/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}] : Connect to the specified server.");
        }
        else if (line.find("/connect ") == 0) {

            // temporary fix to the fact that overwriting the item lots 
            // wont change the items already rendered in the zone the player is
            if (is_player_ingame()) {
                spdlog::warn("Must be in the menu to connect to archipelago.");
                continue;
            }

            std::string param = line.substr(9);
            size_t space_index = param.find(" ");

            if (space_index == std::string::npos) {
                spdlog::info("Missing parameter : Make sure to type '/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}]");
                continue;
            }

            size_t password_index = param.find("password:");
            std::string address = param.substr(0, space_index);
            std::string slot_name = param.substr(space_index + 1, password_index - space_index - 2);
            std::string password = "";
            spdlog::info("{} - {}", address, slot_name, "\n");

            if (password_index != std::string::npos) {
                password = param.substr(password_index + 9);
            }

            setup_apclient(address, slot_name, password);
        }
        else if (line.find("!") == 0) {
            apclient_say(line);
        }
        else {
            spdlog::info("Sorry did not understand that.");
        }
    }
}

void handle_check_locations()
{
    std::list<int32_t> locations_to_check = get_locations_to_check();
    if (locations_to_check.empty()) return;

    for (int32_t location : locations_to_check) {
        // check if we get reward from nashandra
        if (location == 627000) {
            announce_goal_reached();
        }
    }

    check_locations(locations_to_check);
    clear_locations_to_check();
}

void handle_give_items()
{
    if (!is_player_ingame()) return;

    ItemStruct item_struct;
    int num_items = 0;

    for (int i = 0; i < 8; i++) {

        int64_t item_id = get_next_item();
        if (item_id == -1) break;

        Item item;
        item.idk = 0;
        item.durability = -1;
        item.amount = 1;
        item.upgrade = 0;
        item.infusion = 0;

        // item_id < 1000000 means custom item
        if (item_id < 1000000) {
            if (is_statue(item_id)) unpetrify_statue(item_id);

            item.item_id = unused_item_ids[i];
            std::string item_name = get_local_item_name(item_id);
            std::wstring item_name_wide(item_name.begin(), item_name.end());
            set_item_name(item.item_id, item_name_wide);
        }
        else {
            item.item_id = static_cast<int32_t>(item_id);
        }

        item_struct.items[i] = item;
        num_items++;
    }

    if (num_items > 0) {
        give_items(item_struct, num_items);
        confirm_items_given(num_items);
    }
}

void handle_death_link()
{
    if (is_death_link() && is_player_ingame() && player_just_died() && !died_by_deathlink()) {
        send_death_link();
    }
}

void run()
{
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

    spdlog::info("Archipelago client\n"
        "Type '/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}]' to connect to the room\n"
        "Type '/help' for more information\n"
        "-----------------------------------------------------");

    // make sure the archipelago folder exists
    std::filesystem::create_directory("archipelago");
    std::filesystem::create_directory("archipelago/save_data");

    setup_logging();
    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)handle_input, NULL, NULL, NULL);

    force_offline();

    while (true) {
        apclient_poll();

        if (!is_apclient_connected()) continue;

        handle_check_locations();
        handle_give_items();
        handle_death_link();

        Sleep(1000 / 2);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        LoadOriginalDll();
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)run, NULL, NULL, NULL);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        FreeOriginalDll();
    }
    return TRUE;
}