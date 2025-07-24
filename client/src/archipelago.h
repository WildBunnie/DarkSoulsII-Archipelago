#pragma once

#include <string>
#include <set>
#include <list>
#include <vector>

struct APLocationReward {
	int32_t item_id;        // id of the item in archipelago
	int32_t real_item_id;   // id of the item to be given to the player
	std::string item_name;
};

struct APLocation {
	int64_t location_id;
	int32_t reward_amount;
	APLocationReward rewards[10];
};

enum LocationType {
	ItemLotParam2_Chr_Location = 0,
	ItemLotParam2_Other_Location = 1,
	ShopLineupParam_Location = 2
};

void setup_apclient(std::string URI, std::string slot_name, std::string password);
bool is_apclient_connected();
bool is_death_link();
void apclient_poll();
void apclient_say(std::string message);

int64_t get_next_item();
void confirm_items_given(int amount);
void check_locations(std::list<int32_t> locations);
void send_death_link();
bool died_by_deathlink();
std::string get_local_item_name(int32_t item_id);
void announce_goal_reached();
void write_save_file();
void read_save_file();

int32_t get_location_offset(LocationType type);
std::set<int32_t> get_item_bundles();