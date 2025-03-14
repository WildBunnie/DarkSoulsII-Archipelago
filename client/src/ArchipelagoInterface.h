#pragma once

#include "apclient.hpp"
#include "Core.h"
#include <string>

using nlohmann::json;

class CArchipelago {
public:
	BOOL Initialise(std::string URI);
	VOID say(std::string message);
	BOOLEAN isConnected();
	VOID update();
	VOID gameFinished();
	VOID sendDeathLink();
	std::string getItemName(int64_t item_id);

private:
	VOID handleLocationChecks();
};