#include "archipelago.h"

#include "spdlog/spdlog.h"
#include "apclient.hpp"
#include "apuuid.hpp"

#if defined(_M_X64)
	#define GAME_VERSION 0
#elif defined(_M_IX86)
	#define GAME_VERSION 1
#endif

using nlohmann::json;

APClient* ap;

bool fatal_error = false;
bool death_link = false;

enum ItemsHandling {
	NO_ITEMS = 0b000,
	OTHER_WORLDS = 0b001,
	OUR_WORLD = 0b010,
	STARTING_INVENTORY = 0b100,
};

void setup_apclient(std::string URI, std::string slot_name, std::string password)
{
	std::string uuid = ap_get_uuid("uuid");

	if (ap) {
		ap->reset();
	}

	ap = new APClient(uuid, "Dark Souls II", URI);
	fatal_error = false;

	ap->set_room_info_handler([slot_name, password]() {
		int items_handling = ItemsHandling::OTHER_WORLDS | ItemsHandling::STARTING_INVENTORY;
		APClient::Version ap_client_version = { 0, 5, 1 };
		ap->ConnectSlot(slot_name, password, items_handling, {}, ap_client_version);
	});

	ap->set_slot_connected_handler([](const json& data) {
		spdlog::debug("received slot data: {}", data.dump());

		if (data.contains("game_version") && data.at("game_version") != GAME_VERSION) {
			spdlog::warn("The client's game version does not match the version chosen on the yaml file. "
				"Please change to the correct game version or change the yaml file and generate a new game.");
			fatal_error = true;
			return;
		}

		//GameHooks->patchInfiniteTorch();
		if (data.contains("death_link")) {
			death_link = (data.at("death_link") != 0);
			if (death_link) {
				std::list<std::string> tags;
				tags.push_back("DeathLink");
				ap->ConnectUpdate(false, NULL, true, tags);
			}
		}
		//if (data.contains("no_weapon_req") && data.at("no_weapon_req") == 1) {
		//	GameHooks->patchWeaponRequirements();
		//}
		//if (data.contains("no_spell_req") && data.at("no_spell_req") == 1) {
		//	GameHooks->patchSpellRequirements();
		//}

		//GameHooks->locationsToCheck = ap->get_missing_locations();

		//// ask for the items that are in each location
		//std::list<int64_t> locationsList;
		//std::set<int64_t> missingLocations = ap->get_missing_locations();
		//std::set<int64_t> checkedLocations = ap->get_checked_locations();
		//locationsList.insert(locationsList.end(), checkedLocations.begin(), checkedLocations.end());
		//locationsList.insert(locationsList.end(), missingLocations.begin(), missingLocations.end());
		//ap->LocationScouts(locationsList);

		//ap->Get({ Core->getSaveIdKey() });
	});

	ap->set_slot_refused_handler([](const std::list<std::string>& errors) {
		for (const auto& error : errors) {
			spdlog::warn("Connection refused: {}", error);
		}
	});

	ap->set_location_info_handler([](const std::list<APClient::NetworkItem>& items) {
	});

	ap->set_retrieved_handler([](const std::map<std::string, json>& keys) {
		});

	ap->set_items_received_handler([](const std::list<APClient::NetworkItem>& receivedItems) {
		});

	ap->set_bounced_handler([](const json& cmd) {
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
	return ap && ap->get_state() == APClient::State::SLOT_CONNECTED;
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