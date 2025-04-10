#pragma once

#include <string>
#include <set>
#include <list>

void setup_apclient(std::string URI, std::string slot_name, std::string password);

bool is_apclient_connected();

void apclient_poll();
void apclient_say(std::string message);

std::set<int64_t> get_missing_locations();
void check_locations(std::list<int64_t> locations);
std::list<int64_t> get_items_to_give();
void confirm_items_given(int amount);
bool is_death_link();
void send_death_link();
bool died_by_deathlink();
void write_save_file();
void read_save_file();
bool is_save_loaded();
int get_last_received_index();
void set_last_received_index(int value);
std::string get_local_item_name(int32_t item_id);