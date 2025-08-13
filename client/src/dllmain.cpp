#pragma comment(lib, "Crypt32.lib")

#include "archipelago.h"
#include "hooks.h"
#include "game_functions.h"
#include "offsets.h"
#include "ds2.h"
#include "patches.h"
#include "memory.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <conio.h>

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
    static std::string line;

    while (_kbhit()) {
        char ch = _getch();
        if (ch == '\r') { // enter key
            std::cout << std::endl;

            if (line == "/help") {
                spdlog::info("List of available commands : \n"
                    "/help : Prints this help message.\n"
                    "!help : Prints the help message related to Archipelago.\n"
                    "/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}] : Connect to the specified server.");
            }
            else if (line.find("/connect ") == 0) {

                if (is_player_ingame()) {
                    spdlog::warn("Must be in the menu to connect to archipelago.");
                    line.clear();
                    return;
                }

                std::string param = line.substr(9);
                size_t space_index = param.find(" ");
                if (space_index == std::string::npos) {
                    spdlog::info("Missing parameter : Make sure to type '/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}]");
                    line.clear();
                    return;
                }

                size_t password_index = param.find("password:");
                std::string address = param.substr(0, space_index);
                std::string slot_name = param.substr(space_index + 1, password_index == std::string::npos ? std::string::npos : password_index - space_index - 2);
                std::string password = "";

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

            line.clear();
        }
        else if (ch == '\b') { // backspace
            if (!line.empty()) {
                line.pop_back();
                std::cout << "\b \b";
            }
        }
        else {
            line.push_back(ch);
            std::cout << ch;
        }
    }
}

void handle_check_locations()
{
    std::list<int32_t> locations_to_check = get_locations_to_check();
    if (locations_to_check.empty()) return;

    for (int32_t location : locations_to_check) {
        // check if we get reward from nashandra
        if (location == 200627000) {
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

        APItem ap_item = get_archipelago_item(item_id, unused_item_ids[i]);

        Item item;
        item.item_id = ap_item.real_item_id;
        item.idk = 0;
        item.durability = -1;
        item.amount = ap_item.amount;
        item.upgrade = ap_item.reinforcement;
        item.infusion = 0;

        // item_id < 1000000 means custom item
        if (ap_item.item_id < 1000000) {
            if (is_statue(ap_item.item_id)) unpetrify_statue(ap_item.item_id);
            set_item_name(ap_item.real_item_id, ap_item.item_name);
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

    // make sure the folders we need exist
    std::filesystem::create_directory("archipelago");
    std::filesystem::create_directory("archipelago/save_data");

    setup_logging();

    const int tps = 60; // ticks per second
    int loop_counter = 0;
    while (true) {
        handle_input();

        apclient_poll();

        if (!is_apclient_connected()) continue;

        handle_check_locations();
        handle_death_link();

        // artificial delay between giving items
        if (loop_counter >= tps) {
            handle_give_items();
            loop_counter = 0;
        }

        loop_counter++;
        Sleep(1000 / tps);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)run, NULL, NULL, NULL);
    }
    return TRUE;
}