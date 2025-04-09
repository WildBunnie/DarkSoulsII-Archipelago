#pragma once

#include <string>

void setup_apclient(std::string URI, std::string slot_name, std::string password);

bool is_apclient_connected();

void apclient_poll();
void apclient_say(std::string message);
