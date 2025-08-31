#include "archipelago.h"

#include "patches.h"
#include "params.h"
#include "memory.h"
#include "randomizer.h"
#include "hooks.h"
#include "game_functions.h"
#include "ds2.h"

#include "spdlog/spdlog.h"

#include "apuuid.hpp"

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

#include <string>

#if defined(_M_X64)
	#define GAME_VERSION 0
#elif defined(_M_IX86)
	#define GAME_VERSION 1
#endif

using nlohmann::json;

enum ItemsHandling {
	NO_ITEMS = 0b000,
	OTHER_WORLDS = 0b001,
	OUR_WORLD = 0b010,
	STARTING_INVENTORY = 0b100,
};

void reset_state(APState& state)
{
	// Clean up APClient if we own it
	if (state.ap) {
		delete state.ap;
		state.ap = nullptr;
	}

	// Reset slot data
	state.slot_data = APSlotData{};

	// Reset primitive members
	state.last_received_index = -1;
	state.fatal_error = false;
	state.died_by_deathlink = false;
	state.player_seed.clear();

	// Reset containers
	state.location_map.clear();
	state.item_names.clear();
	while (!state.item_queue.empty()) state.item_queue.pop();
	while (!state.location_queue.empty()) state.location_queue.pop();
}

void setup_apclient(APState& state, const std::string URI, const std::string slot_name, const std::string password)
{
	std::string uuid = ap_get_uuid("uuid");

	if (state.ap) reset_state(state);
	state.ap = new APClient(uuid, "Dark Souls II", URI);

	state.ap->set_room_info_handler([&state, slot_name, password]() {
		int items_handling = ItemsHandling::OTHER_WORLDS | ItemsHandling::STARTING_INVENTORY;
		APClient::Version ap_client_version = { 0, 5, 1 };
		state.ap->ConnectSlot(slot_name, password, items_handling, {}, ap_client_version);
	});

	state.ap->set_slot_connected_handler([&state](const json& data) {
		spdlog::debug("received slot data: {}", data.dump());

		if (data.contains("game_version") && data.at("game_version") != GAME_VERSION) {
			spdlog::error("The client's game version does not match the version chosen on the yaml file. "
				"Please change to the correct game version or change the yaml file and generate a new game.");
			state.fatal_error = true;
			return;
		}

		// load data we save about archipelago
		if (!read_save_file(state)) {
			if (!write_save_file(state)) {
				state.fatal_error = true;
				spdlog::error("Fatal error: unable to read or write Archipelago save data. "
					"Check file permissions and disk space.");
				return;
			}
		}

		// we do this so different players in the same room dont have the same items
		state.player_seed = state.ap->get_seed() + state.ap->get_player_alias(state.ap->get_player_number()) + std::to_string(state.ap->get_team_number());

		// apply patches
		patch_infinite_torch();

		if (data.contains("death_link")) {
			bool death_link = data.at("death_link") != 0;
			state.slot_data.death_link = death_link;

			if (death_link) {
				state.ap->ConnectUpdate(false, NULL, true, {"DeathLink"});
			}
		}

		if (data.contains("no_weapon_req") && data.at("no_weapon_req") == 1) {
			remove_weapon_requirements();
		}
		if (data.contains("no_spell_req") && data.at("no_spell_req") == 1) {
			remove_spell_requirements();
		}
		if (data.contains("no_armor_req") && data.at("no_armor_req") == 1) {
			remove_armor_requirements();
		}
		if (data.contains("no_equip_load") && data.at("no_equip_load") == 1) {
			remove_weight_from_params();
		}

		if (data.contains("autoequip") && data.at("autoequip") == 1) {
			state.slot_data.auto_equip = true;
		}
		if (data.contains("ap_to_location_id")) {
			for (auto& [key, val] : data.at("ap_to_location_id").items())
			{
				int32_t int_key = std::stoi(key);
				int32_t int_val = val.get<int32_t>();
				state.slot_data.ap_to_location_id[int_key] = int_val;
			}
		}
		if (data.contains("location_to_ap_id")) {
			for (auto& [key, val] : data.at("location_to_ap_id").items()) {
				int32_t int_key = std::stoi(key);

				std::vector<int32_t> ids;
				for (const auto& id : val) {
					ids.push_back(id.get<int32_t>());
				}

				state.slot_data.location_to_ap_id[int_key] = std::move(ids);
			}
		}
		if (data.contains("reinforcements")) {
			for (auto& [key, val] : data.at("reinforcements").items())
			{
				int32_t item_id = std::stoi(key);
				int8_t reinforcement = val.get<int8_t>();
				state.slot_data.reinforcements[item_id] = reinforcement;
			}
		}
		if (data.contains("item_bundles")) {
			for (const auto& id : data.at("item_bundles")) {
				state.slot_data.item_bundles.insert(id.get<int32_t>());
			}
		}
		if (data.contains("keep_unrandomized")) {
			for (const auto& id : data.at("keep_unrandomized")) {
				state.slot_data.locations_to_ignore.insert(id.get<int32_t>());
			}
		}
		if (data.contains("randomize_starting_loadout") && data.at("randomize_starting_loadout") == 1) {
			if (data.contains("starting_weapon_requirement")) {
				ClassRandomizationFlag flag = static_cast<ClassRandomizationFlag>(data.at("starting_weapon_requirement"));
				randomize_starter_classes(state.player_seed, flag);
			}
		}

		std::list<int64_t> locations_list;
		std::set<int64_t> missing_locations = state.ap->get_missing_locations();
		std::set<int64_t> checked_locations = state.ap->get_checked_locations();
		locations_list.insert(locations_list.end(), missing_locations.begin(), missing_locations.end());
		locations_list.insert(locations_list.end(), checked_locations.begin(), checked_locations.end());
		state.ap->LocationScouts(locations_list);
	});

	state.ap->set_slot_refused_handler([](const std::list<std::string>& errors) {
		for (const auto& error : errors) {
			spdlog::warn("connection refused: {}", error);
		}
	});

	state.ap->set_location_info_handler([&state](const std::list<APClient::NetworkItem>& items) {
		for (const auto& item : items) {
			int32_t _archipelago_address = item.location;
			int32_t _location_id = state.slot_data.ap_to_location_id[_archipelago_address];

			APLocation& location = state.location_map[_location_id]; // inserts if not present
			assert(location.reward_amount < 10 && "location can't have more than 10 rewards");
			location.location_id = _location_id;

			// index with reward_amount so that two custom
			// items dont have the same real_item_id
			int32_t custom_item_id = unused_item_ids[location.reward_amount];

			// use a different custom item for shops
			if (location.location_id >= 300'000'000)
			{
				custom_item_id = custom_shop_item_id;
			}

			APItem reward = get_archipelago_item(state.ap, state.slot_data, item.item, custom_item_id, item.player);
			location.rewards[location.reward_amount++] = reward;
		}

		override_item_params(state);
		init_hooks(state);
	});

	state.ap->set_items_received_handler([&state](const std::list<APClient::NetworkItem>& received_items) {
		for (const auto& item : received_items) {
			if (item.index <= state.last_received_index) continue;
			state.item_queue.push(item.item);
		}
	});

	state.ap->set_bounced_handler([&state](const json& cmd) {
		if (!state.slot_data.death_link) return;

		auto tagsIt = cmd.find("tags");
		auto dataIt = cmd.find("data");

		if (tagsIt != cmd.end() && tagsIt->is_array() && std::find(tagsIt->begin(), tagsIt->end(), "DeathLink") != tagsIt->end()) {
			if (dataIt != cmd.end() && dataIt->is_object()) {
				json data = *dataIt;

				std::string source = data["source"].is_string() ? data["source"].get<std::string>() : "";
				std::string cause = data["cause"].is_string() ? data["cause"].get<std::string>() : "";

				if (!source.empty() && source != state.ap->get_slot()) {
					spdlog::info("DeathLink Received | Source: {} Caused by: {}", source, cause);
					if (kill_player()) {
						state.died_by_deathlink = true;
					};
				}
			}
		}
	});

	state.ap->set_print_handler([&state](const std::string& msg) {
		if (state.fatal_error) return;
		spdlog::info(msg);
	});

	state.ap->set_print_json_handler([&state](const std::list<APClient::TextNode>& msg) {
		if (state.fatal_error) return;
		auto message = state.ap->render_json(msg, APClient::RenderFormat::TEXT);
		spdlog::info(message);
	});
}

bool is_apclient_connected(APClient* ap)
{
	return ap && ap->get_state() == APClient::State::SLOT_CONNECTED;
}

void apclient_poll(APClient* ap)
{
	if (ap) ap->poll();
}

void apclient_say(APClient* ap, std::string message)
{
	if (ap && is_apclient_connected(ap)) {
		ap->Say(message);
	}
}

void check_locations(APClient* ap, APSlotData slot_data, std::list<int32_t> locations)
{
	std::list<int64_t> checks;

	for (auto& location : locations)
	{
		if (slot_data.location_to_ap_id.contains(location))
		{
			for (auto& ap_id : slot_data.location_to_ap_id[location]) {
				checks.push_back(static_cast<int64_t>(ap_id));
			}
		}
	}
	
	if (!checks.empty()) {
		ap->LocationChecks(checks);
	}
}

void send_death_link(APClient* ap)
{
	if (!is_apclient_connected(ap)) return;

	spdlog::info("Sending DeathLink");

	json data{
		{"time", ap->get_server_time()},
		{"cause", "Skill Issue"},
		{"source", ap->get_slot()},
	};
	ap->Bounce(data, {}, {}, { "DeathLink" });
}

std::string get_local_item_name(APClient* ap, int32_t item_id)
{
	return ap->get_item_name(item_id, ap->get_player_game(ap->get_player_number()));
}

void announce_goal_reached(APClient* ap)
{
    if (ap) ap->StatusUpdate(APClient::ClientStatus::GOAL);
}

std::string get_save_data_file(APState& state)
{
	return "archipelago/save_data/" + std::string(state.ap->get_slot()) + "_" + std::string(state.ap->get_seed()) + ".json";
}

bool write_save_file(APState& state)
{
	try {
		json j = {
			{"last_received_index", state.last_received_index}
		};

		std::ofstream file(get_save_data_file(state));

		file << j.dump(4);

		file.close();

		spdlog::debug("Successfully wrote save file with last_received_index: {}", state.last_received_index);
		return true;
	}
	catch (const std::exception& e) {
		spdlog::warn("Error writing save file");
		return false;
	}
}

bool read_save_file(APState& state)
{
	try {
		std::ifstream file(get_save_data_file(state));

		json j;
		file >> j;

		if (j.contains("last_received_index")) {
			state.last_received_index = j["last_received_index"];
			spdlog::debug("Successfully read last_received_index: {}", state.last_received_index);
		}

		file.close();
		return true;
	}
	catch (const std::exception& e) {
		spdlog::warn("Error reading save file");
		return false;
	}
}

APItem get_archipelago_item(APClient* ap, APSlotData slot_data, int32_t item_id, int custom_item_id, int player)
{
	if (player == -1) {
		player = ap->get_player_number();
	}
	if (custom_item_id == -1) {
		custom_item_id = unused_item_ids[0];
	}

	APItem reward;
	if (player == ap->get_player_number()) {
		// if the id is less than 1000000 it's a custom item
		if (item_id < 1000000) {
			reward.real_item_id = custom_item_id;
			reward.item_id = item_id;

			std::string item_name = ap->get_item_name(item_id, ap->get_player_game(player));
			std::wstring item_name_wide(item_name.begin(), item_name.end());
			reward.item_name = item_name_wide;
		}
		else {
			reward.item_id = item_id;
			reward.real_item_id = item_id;
		}
	}
	else {
		reward.item_id = custom_item_id;
		reward.real_item_id = custom_item_id;

		std::string player_name = ap->get_player_alias(player);
		std::string item_name = ap->get_item_name(item_id, ap->get_player_game(player));
		std::string full_name = player_name + "'s " + item_name;
		std::wstring full_name_wide(full_name.begin(), full_name.end());
		reward.item_name = full_name_wide;
	}

	// convert bundle item id to the item with the amount
	if (slot_data.item_bundles.contains(reward.item_id)) {
		int bundle_amount = reward.item_id % 1000;
		reward.real_item_id -= bundle_amount;
		reward.amount = bundle_amount;
	}
	else {
		reward.amount = 1;
	}

	if (slot_data.reinforcements.contains(reward.real_item_id)) {
		reward.reinforcement = slot_data.reinforcements[reward.real_item_id];
	}
	else {
		reward.reinforcement = 0;
	}

	return reward;
}