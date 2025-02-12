#pragma once

#include "apclient.hpp"
#include "Core.h"

using nlohmann::json;

class CArchipelago {
public:
	BOOL Initialise(std::string URI);
	VOID say(std::string message);
	BOOLEAN isConnected();
	VOID update();
	VOID gameFinished();
	VOID sendDeathLink();
};