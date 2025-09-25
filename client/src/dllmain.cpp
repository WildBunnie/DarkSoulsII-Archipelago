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

void handle_input(APState& state)
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
                    "/connect {SERVER_IP}:{SERVER_PORT} {SLOT_NAME} [password:{PASSWORD}] : Connect to the specified server.\n"
                    "!{message} : Send a message to Archipelago.\n"
                    "/reset : Reset your progress to receive all items you have been sent again.\n"
                    "/check_index : See the index of the last received item.\n"
                    "/decrement_index {number} : Receive the last {number} items you have been sent again. The number cannot go below -1.");
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

                setup_apclient(state, address, slot_name, password);
            }
            else if (line == "/reset") {
                state.last_received_index = -1;
                spdlog::info("The value of last_received_index has been reset. You will receive all items again.");
                state.ap->Sync();
            }
            else if (line == "/check_index") {
                spdlog::info("Current last_received_index: {}", state.last_received_index);
            }
            else if (line.find("/decrement_index ") == 0) {
                std::string param = line.substr(17);
                try {
                    int decrement_value = std::stoi(param);
                    state.last_received_index -= decrement_value;

                    if (state.last_received_index < -1) {
                        state.last_received_index = -1;
                    }

                    spdlog::info("Decremented last_received_index by {}. New value: {}", decrement_value, state.last_received_index);
                    state.ap->Sync();
                }
                catch (const std::exception& e) {
                    spdlog::warn("Invalid number provided. Usage: /decrement_index {number}");
                }
            }
            else if (line.find("!") == 0) {
                apclient_say(state.ap, line);
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

void handle_check_locations(APState& state)
{
    if (state.location_queue.empty()) return;

    std::list<int32_t> locations_to_check;
    while (!state.location_queue.empty()) {
        int32_t location = state.location_queue.front();
        state.location_queue.pop();

        // TODO: make this less scuffed
        if (location == 200627000) {
            announce_goal_reached(state.ap);
        }

        locations_to_check.push_back(location);
    }

    check_locations(state.ap, state.slot_data, locations_to_check);
}

void handle_give_items(APState& state)
{
    if (!is_player_ingame()) return;

    ItemStruct item_struct;
    int num_items = 0;

    for (int i = 0; i < 8; i++) {

        if (state.item_queue.empty()) break;
        int64_t item_id = state.item_queue.front();
        state.item_queue.pop();

        APItem ap_item = get_archipelago_item(state.ap, state.slot_data, item_id, unused_item_ids[i]);

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
            state.item_names[ap_item.real_item_id] = ap_item.item_name;
        }

        item_struct.items[i] = item;
        num_items++;
    }

    if (num_items > 0) {
        give_items(item_struct, num_items);
        state.last_received_index += num_items;
        write_save_file(state);
    }
}

void handle_death_link(APState& state)
{
    if (state.slot_data.death_link && is_player_ingame() && player_just_died()) {
        if (!state.died_by_deathlink) {
            send_death_link(state.ap);
        }
        else {
            state.died_by_deathlink = false;
        }
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

    APState state;

    const int tps = 60; // ticks per second
    int loop_counter = 0;
    while (true) {
        handle_input(state);

        apclient_poll(state.ap);

        if (state.fatal_error) continue;
        if (!is_apclient_connected(state.ap)) continue;

        handle_check_locations(state);
        handle_death_link(state);

        // artificial delay between giving items
        if (loop_counter >= tps) {
            handle_give_items(state);
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