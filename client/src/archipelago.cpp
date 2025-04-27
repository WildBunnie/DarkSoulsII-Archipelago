#include "archipelago.h"

#include "patches.h"
#include "memory.h"
#include "randomizer.h"
#include "hooks.h"
#include "game_functions.h"
#include "ds2.h"

#include "spdlog/spdlog.h"

#include "apclient.hpp"
#include "apuuid.hpp"

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

#include <map>
#include <queue>
#include <string>

#if defined(_M_X64)
	#define GAME_VERSION 0
#elif defined(_M_IX86)
	#define GAME_VERSION 1
#endif

using nlohmann::json;

APClient* ap;

std::string room_id_key;
std::string room_id_value;
std::string player_seed;
bool fatal_error = false;
bool death_link = false;
bool autoequip = false;
bool save_loaded = false;
bool _died_by_deathlink = false;
int last_received_index = -1;
std::set<int32_t> locations_to_ignore;
std::queue<APClient::NetworkItem> items_to_give;

enum ItemsHandling {
	NO_ITEMS = 0b000,
	OTHER_WORLDS = 0b001,
	OUR_WORLD = 0b010,
	STARTING_INVENTORY = 0b100,
};

void reset_apclient()
{
	ap->reset();
	room_id_key.clear();
	room_id_value.clear();
	death_link = false;
	save_loaded = false;
	_died_by_deathlink = false;
	last_received_index = 0;
	locations_to_ignore.clear();
	while (!items_to_give.empty()) {
		items_to_give.pop();
	}
}

void setup_apclient(std::string URI, std::string slot_name, std::string password)
{
	std::string uuid = ap_get_uuid("uuid");

	if (ap) reset_apclient();

	ap = new APClient(uuid, "Dark Souls II", URI);
	fatal_error = false;

	ap->set_room_info_handler([slot_name, password]() {
		int items_handling = ItemsHandling::OTHER_WORLDS | ItemsHandling::STARTING_INVENTORY;
		APClient::Version ap_client_version = { 0, 5, 1 };
		ap->ConnectSlot(slot_name, password, items_handling, {}, ap_client_version);
	});

	ap->set_slot_connected_handler([](const json& data) {
		spdlog::debug("received slot data: {}", data.dump());

		// we do this so different players in the same room dont have the same items
		player_seed = ap->get_seed() + ap->get_player_alias(ap->get_player_number()) + std::to_string(ap->get_team_number());

		if (data.contains("game_version") && data.at("game_version") != GAME_VERSION) {
			spdlog::error("The client's game version does not match the version chosen on the yaml file. "
				"Please change to the correct game version or change the yaml file and generate a new game.");
			fatal_error = true;
			return;
		}

		if (data.contains("death_link")) {
			death_link = data.at("death_link") != 0;
			if (death_link) {
				std::list<std::string> tags;
				tags.push_back("DeathLink");
				ap->ConnectUpdate(false, NULL, true, tags);
			}
		}

		uintptr_t base_address = get_base_address();

		patch_infinite_torch(base_address);
		patch_unbreakable_chests(base_address);
		if (data.contains("no_weapon_req") && data.at("no_weapon_req") == 1) {
			patch_weapon_requirements(base_address);
		}
		if (data.contains("no_spell_req") && data.at("no_spell_req") == 1) {
			patch_spell_requirements(base_address);
		}
		if (data.contains("no_equip_load") && data.at("no_equip_load") == 1) {
			patch_no_equip_load(base_address);
		}
		if (data.contains("autoequip") && data.at("autoequip") == 1) {
			autoequip = true;
		}

		locations_to_ignore.insert(1700000); // estus flask from emerald herald
		if (data.contains("infinite_lifegems") && data.at("infinite_lifegems") == 1) {
			locations_to_ignore.insert(75400601);
		}

		if (data.contains("randomize_starting_loadout") && data.at("randomize_starting_loadout") == 1) {
			if (data.contains("starting_weapon_requirement")) {
				ClassRandomizationFlag flag = static_cast<ClassRandomizationFlag>(data.at("starting_weapon_requirement"));
				randomize_starter_classes(player_seed, flag);
			}
		}

		// we store an uuid we create in the server's data storage
		// this allows us to have an unique identifier for each room
		room_id_key = "roomid_" + std::to_string(ap->get_team_number()) + "_" + ap->get_slot();
		ap->Get({ room_id_key });

		std::list<int64_t> locationsList;
		std::set<int64_t> missingLocations = ap->get_missing_locations();
		std::set<int64_t> checkedLocations = ap->get_checked_locations();
		locationsList.insert(locationsList.end(), checkedLocations.begin(), checkedLocations.end());
		locationsList.insert(locationsList.end(), missingLocations.begin(), missingLocations.end());
		ap->LocationScouts(locationsList);
	});

	ap->set_slot_refused_handler([](const std::list<std::string>& errors) {
		for (const auto& error : errors) {
			spdlog::warn("connection refused: {}", error);
		}
	});

	ap->set_retrieved_handler([](const std::map<std::string, json>& keys) {
		if (keys.contains(room_id_key) && !keys.at(room_id_key).is_null()) {
			room_id_value = keys.at(room_id_key);
			spdlog::debug("retrieved room id: {}", room_id_value);
			read_save_file();
		}
		else {
			boost::uuids::uuid uuid = boost::uuids::random_generator()();
			room_id_value = boost::uuids::to_string(uuid);
			if (ap->Set(room_id_key, room_id_value, false, {})) {
				spdlog::debug("new room id has been set {}", room_id_value);
				write_save_file();
			}
			else {
				spdlog::error("error setting room id");
				fatal_error = true;
			}
		}
		save_loaded = true;
		
		// forces the server to send us all the items
		// it's a shitty solution to the fact that
		// we get items before the save is loaded
		ap->Sync();
	});

	ap->set_location_info_handler([](const std::list<APClient::NetworkItem>& items) {
		std::map<int32_t, int32_t> location_rewards;
		std::map<int32_t, int32_t> custom_items;
		std::map<int32_t, std::string> reward_names;
		for (const auto& item : items) {
			if (item.player == ap->get_player_number()) {
				// if the id is less than 1000000 it's a custom item
				if (item.item < 1000000) {
					location_rewards[item.location] = the_item_id;
					custom_items[item.location] = item.item;

					std::string item_name = ap->get_item_name(item.item, ap->get_player_game(item.player));
					reward_names[item.location] = item_name;
				}
				else {
					location_rewards[item.location] = item.item;
				}
			}
			else {
				location_rewards[item.location] = the_item_id;

				std::string player_name = ap->get_player_alias(item.player);
				std::string item_name = ap->get_item_name(item.item, ap->get_player_game(item.player));
				reward_names[item.location] = player_name + "' " + item_name;
			}
		}
		override_item_params(location_rewards, player_seed, locations_to_ignore);
		init_hooks(reward_names, custom_items, autoequip);
	});

	ap->set_items_received_handler([](const std::list<APClient::NetworkItem>& received_items) {
		if (!save_loaded) {
			return;
		}
		for (const auto& item : received_items) {
			if (item.index <= last_received_index) continue;
			items_to_give.push(item);
		}
	});

	ap->set_bounced_handler([](const json& cmd) {
		if (death_link) {
			auto tagsIt = cmd.find("tags");
			auto dataIt = cmd.find("data");

			if (tagsIt != cmd.end() && tagsIt->is_array() && std::find(tagsIt->begin(), tagsIt->end(), "DeathLink") != tagsIt->end()) {
				if (dataIt != cmd.end() && dataIt->is_object()) {
					json data = *dataIt;

					std::string source = data["source"].is_string() ? data["source"].get<std::string>() : "";
					std::string cause = data["cause"].is_string() ? data["cause"].get<std::string>() : "";

					if (!source.empty() && source != ap->get_slot()) {
						spdlog::info("DeathLink Received | Source: {} Caused by: {}", source, cause);
						if (kill_player()) {
							_died_by_deathlink = true;
						};
					}
				}
			}
		}
	});

	ap->set_print_handler([](const std::string& msg) {
		if (fatal_error) return;
		spdlog::info(msg);
	});

	ap->set_print_json_handler([](const std::list<APClient::TextNode>& msg) {
		if (fatal_error) return;
		auto message = ap->render_json(msg, APClient::RenderFormat::TEXT);
		spdlog::info(message);
	});
}

bool is_apclient_connected() 
{
	return ap && ap->get_state() == APClient::State::SLOT_CONNECTED && save_loaded;
}

bool is_death_link()
{
	return death_link;
}

void apclient_poll()
{
	if (ap && !fatal_error) ap->poll();
}

void apclient_say(std::string message)
{
	if (ap && is_apclient_connected() && !fatal_error) {
		ap->Say(message);
	}
}

int64_t get_next_item()
{
	if (!items_to_give.empty()) {
		int64_t item = items_to_give.front().item;
		items_to_give.pop();
		return item;
	}
	return -1;
}

void confirm_items_given(int amount)
{
	last_received_index += amount;
	write_save_file();
}

void check_locations(std::list<int32_t> locations)
{
	std::list<int64_t> checks;
	std::set<int64_t> missing_locations = ap->get_missing_locations();

	for (auto& location : locations)
	{
		if (missing_locations.contains(location))
		{
			checks.push_back(static_cast<int64_t>(location));
		}
	}
	
	if (!checks.empty()) {
		ap->LocationChecks(checks);
	}
}

void send_death_link()
{
	if (!is_apclient_connected() || !death_link) return;

	spdlog::info("Sending DeathLink");

	json data{
		{"time", ap->get_server_time()},
		{"cause", "Dark Souls II"},
		{"source", ap->get_slot()},
	};
	ap->Bounce(data, {}, {}, { "DeathLink" });
}

bool died_by_deathlink()
{
	if (_died_by_deathlink) {
		_died_by_deathlink = false;
		return true;
	}
	return false;
}

std::string get_local_item_name(int32_t item_id)
{
	return ap->get_item_name(item_id, ap->get_player_game(ap->get_player_number()));
}

void announce_goal_reached()
{
    if (ap) ap->StatusUpdate(APClient::ClientStatus::GOAL);
}

void write_save_file()
{
	try {
		json j = {
			{"last_received_index", last_received_index}
		};

		std::ofstream file("archipelago/save_data/" + std::string(room_id_value) + "_" + std::string(ap->get_slot()) + ".json");

		file << j.dump(4);

		file.close();

		spdlog::debug("Successfully wrote save file");
	}
	catch (const std::exception& e) {
		spdlog::debug("Error writing save file");
	}
}

void read_save_file()
{
	try {
		std::ifstream file("archipelago/save_data/" + std::string(room_id_value) + "_" + std::string(ap->get_slot()) + ".json");

		json j;
		file >> j;

		if (j.contains("last_received_index")) {
			last_received_index = j["last_received_index"];
			spdlog::debug("Successfully read last_received_index: {}", last_received_index);
		}

		file.close();
	}
	catch (const std::exception& e) {
		spdlog::debug("Error reading save file");
	}
}