#pragma once

#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <queue>

#include "apclient.hpp"

struct APItem {
	int32_t item_id;        // id of the item in archipelago
	int32_t real_item_id;   // id of the item to be given to the player
	std::wstring item_name;
	int32_t amount;
	int8_t reinforcement;
};

struct APLocation {
	int64_t location_id;
	int32_t reward_amount;
	APItem rewards[10];
};

struct APSlotData {
	bool death_link = false;
	bool auto_equip = false;

	std::unordered_map<int32_t, int32_t> ap_to_location_id;
	std::unordered_map<int32_t, std::vector<int32_t>> location_to_ap_id;
	std::unordered_map<int32_t, int8_t> reinforcements;

	std::set<int32_t> locations_to_ignore;
	std::set<int32_t> item_bundles;
};

struct APState {
    APClient* ap = nullptr;
	APSlotData slot_data;

	int last_received_index = -1;

	bool fatal_error = false;
	bool died_by_deathlink = false;

	std::string player_seed;

	std::unordered_map<int32_t, APLocation> location_map;
	std::unordered_map<int32_t, std::wstring> item_names;

	std::queue<int32_t> item_queue;
	std::queue<int32_t> location_queue;
};

void setup_apclient(APState& state, const std::string URI, const std::string slot_name, const std::string password);

bool is_apclient_connected(APClient* ap);
void apclient_poll(APClient* ap);
void apclient_say(APClient* ap, std::string message);
void send_death_link(APClient* ap);
void announce_goal_reached(APClient* ap);
void check_locations(APClient* ap, APSlotData slot_data, std::list<int32_t> locations);

bool write_save_file(APState& state);
bool read_save_file(APState& state);
std::string get_local_item_name(APClient* ap, int32_t item_id);
APItem get_archipelago_item(APClient* ap, APSlotData slot_data, int32_t item_id, int custom_item_id = -1, int player = -1);