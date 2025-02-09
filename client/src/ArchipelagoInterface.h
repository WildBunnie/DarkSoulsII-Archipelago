#pragma once

#include "apclient.hpp"


using nlohmann::json;

class CArchipelago {
public:
	BOOL Initialise(std::string URI, std::string slotName);
	VOID say(std::string message);
	BOOLEAN isConnected();
	VOID update();
	VOID gameFinished();
};