#pragma comment(lib, "Crypt32.lib")

#include "dinput8/dinput8.h"
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
        if (location == 627000) {
            announce_goal_reached();
        }
    }

    check_locations(locations_to_check);
    clear_locations_to_check();
}

std::set<int64_t> item_bundles = { 50880002, 50880003, 50880005, 60160005, 50880010, 60800010, 60010002, 60010003, 60800020, 60010005, 60970002, 60970003, 60010008, 60970005, 60970010, 60800030, 61140002, 60180003, 60010020, 60820015, 60030003, 60030004, 60030005, 60350003, 60030007, 60350005, 60820020, 60200002, 61160002, 60050002, 60370002, 60370004, 60370005, 60370010, 60540007, 60540010, 60070002, 60070003, 60540020, 60710003, 61030002, 61030003, 61030008, 60625020, 50960002, 60240003, 53200004, 60560004, 53200006, 60560005, 60880010, 60090002, 60090003, 60410002, 60410003, 60410005, 60880020, 60239002, 60880030, 60260003, 60580003, 60260005, 60580010, 60900010, 60900015, 60110002, 60238002, 60238003, 60430002, 60430003, 60430005, 60900020, 61070003, 60430010, 60900030, 60280003, 60600005, 60237002, 60237003, 60920010, 60600015, 60920015, 60900050, 61090002, 60450004, 60600020, 60920020, 62050003, 60151002, 60151003, 60151004, 60151005, 60151006, 60450010, 60450015, 60770010, 60236002, 60236003, 60620002, 60620003, 60620004, 60620005, 60770020, 60770030, 61110003, 60235002, 60235003, 60235005, 60320003, 60320005, 60320008, 60320010, 61130002, 60575002, 60511003, 60575003, 60575005, 60810015, 60020002, 60020003, 60020004, 60020005, 60980002, 60980003, 60020008, 60105002, 60105003, 60810030, 61150002, 60531002, 60531003, 60595002, 60531005, 60595003, 60830015, 60040002, 60040003, 60595010, 60040005, 61000002, 61000003, 61000005, 60530003, 60530005, 60060002, 60060003, 60060004, 60850020, 60550005, 60870020, 50885003, 50885005, 50885010, 60570003, 60250004, 60250006, 60527002, 60527003, 60975002, 60975003, 60975005, 60975007, 60036002, 60036003, 60036004, 60036005, 60100003, 60100005, 60420002, 60420003, 60420004, 60420005, 60270003, 60035002, 60035003, 60590010, 60590015, 60120002, 60120003, 60590020, 60910020, 61080002, 61060002, 60290003, 60610003, 60290005, 60610004, 60610005, 60930010, 62060002, 62060003, 60930020, 62060005, 60930030, 60780015, 60310002, 60310003, 60780020, 60310005, 60310008, 60780030 };
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

        if (item_bundles.contains(item_id)) {
            int bundle_amount = item_id % 1000;
            item.amount = bundle_amount;
            item_id = item_id - bundle_amount;
        }

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

    // make sure the folders we need exist
    std::filesystem::create_directory("archipelago");
    std::filesystem::create_directory("archipelago/save_data");
    std::filesystem::create_directory("archipelago/textures");

    setup_logging();
    force_offline();

    uintptr_t base_address = get_base_address();
    patch_qol(base_address);

    const int tps = 60; // ticks per second
    int loop_counter = 0;
    while (true) {
        handle_input();

        apclient_poll();

        if (!is_apclient_connected()) continue;

        handle_check_locations();
        handle_death_link();

        // artifical delay between giving items
        // to prevent items not giving
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
        LoadOriginalDll();
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)run, NULL, NULL, NULL);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        FreeOriginalDll();
    }
    return TRUE;
}