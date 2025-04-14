#pragma once

#include <string>
#include <set>
#include <list>

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