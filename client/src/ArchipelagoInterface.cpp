#include "ArchipelagoInterface.h"
#include "apuuid.hpp"
#include "hooks.h"
#include <spdlog/spdlog.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#ifdef __EMSCRIPTEN__
#define DATAPACKAGE_CACHE "/settings/datapackage.json"
#define UUID_FILE "/settings/uuid"
#else
#define DATAPACKAGE_CACHE "datapackage.json" // TODO: place in %appdata%
#define UUID_FILE "uuid" // TODO: place in %appdata%
#endif

bool ap_sync_queued = false;
APClient* ap;
APClient::Version APClientVersion = { 0, 5, 1 };
extern ClientCore* Core;
extern Hooks* GameHooks;

BOOL CArchipelago::Initialise(std::string URI) {
	std::string uuid = ap_get_uuid(UUID_FILE);
	if (ap != nullptr) {
		ap->reset();
	}

	ap = new APClient(uuid, "Dark Souls II", URI);

	ap_sync_queued = false;
	ap->set_socket_connected_handler([]() {
	});

	ap->set_socket_disconnected_handler([]() {
	});

	ap->set_slot_connected_handler([](const json& data) {
		spdlog::info("Slot connected successfully, reading slot data ... ");
		spdlog::debug(data.dump());

		if (data.contains("death_link")) {
			GameHooks->isDeathLink = (data.at("death_link") != 0);
		}
		if (data.contains("game_version")) {
			GameVersion chosenVersion = static_cast<GameVersion>(data.at("game_version"));;
			if (chosenVersion != Core->gameVersion) {
				Core->Panic("The client's game version does not match the version chosen on the config file, " 
							"please change to the correct version or change the config file and generate a new game.");
				return;
			}
		}
		if (data.contains("no_weapon_req") && data.at("no_weapon_req") == 1) {
			GameHooks->patchWeaponRequirements();
		}

		std::list<std::string> tags;
		if (GameHooks->isDeathLink) {
			tags.push_back("DeathLink");
			ap->ConnectUpdate(false, 1, true, tags);
		}

		Core->pSeed = ap->get_seed();
		Core->pTeamNumber = ap->get_team_number();

		GameHooks->locationsToCheck = ap->get_missing_locations();

		// ask for the items that are in each location
		std::list<int64_t> locationsList;
		std::set<int64_t> missingLocations = ap->get_missing_locations();
		std::set<int64_t> checkedLocations = ap->get_checked_locations();
		locationsList.insert(locationsList.end(), checkedLocations.begin(), checkedLocations.end());
		locationsList.insert(locationsList.end(), missingLocations.begin(), missingLocations.end());
		ap->LocationScouts(locationsList);

		ap->Get({ Core->getSaveIdKey() });
	});

	ap->set_retrieved_handler([](const std::map<std::string, json>& keys) {
		if (keys.contains(Core->getSaveIdKey()) && !keys.at(Core->getSaveIdKey()).is_null()) {
			Core->pSaveId = keys.at(Core->getSaveIdKey());
			spdlog::debug("retrieved save id {}", Core->pSaveId);
			Core->ReadSaveFile();
		}
		else {
			boost::uuids::uuid uuid = boost::uuids::random_generator()();
			Core->pSaveId = boost::uuids::to_string(uuid);
			if (ap->Set(Core->getSaveIdKey(), Core->pSaveId, false, {})) {
				spdlog::debug("new save id has been set {}", Core->pSaveId);
				Core->WriteSaveFile();
				Core->saveLoaded = true;
			}
			else {
				spdlog::error("error setting save id");
			}
		}
	});

	// response to the ap->LocationScouts
	// contains the items that are in each location
	ap->set_location_info_handler([](const std::list<APClient::NetworkItem>& items) {
		for (const auto& item : items) {
			std::string player_name = ap->get_player_alias(item.player);
			std::string item_name = ap->get_item_name(item.item, ap->get_player_game(item.player));
			bool isLocal = player_name == Core->pSlotName;

			GameHooks->locationRewards[item.location] = {item.item, item_name, player_name, isLocal};
		}
		GameHooks->overrideShopParams();
	});
	
	ap->set_slot_disconnected_handler([]() {
		spdlog::info("Slot disconnected");
	});

	ap->set_slot_refused_handler([](const std::list<std::string>& errors) {
		for (const auto& error : errors) {
			spdlog::warn("Connection refused: {}", error);
		}
	});

	ap->set_room_info_handler([]() {
		ap->ConnectSlot(Core->pSlotName, Core->pPassword, 3, {}, APClientVersion);
	});

	ap->set_items_received_handler([](const std::list<APClient::NetworkItem>& receivedItems) {

		if (!ap->is_data_package_valid()) {
			// NOTE: this should not happen since we ask for data package before connecting
			if (!ap_sync_queued) ap->Sync();
			ap_sync_queued = true;
			return;
		}

		for (auto const& i : receivedItems) {
			Core->itemsToGive.push_back(i);
		}

	});

	//TODO :   * you can still use `set_data_package` or `set_data_package_from_file` during migration to make use of the old cache

	ap->set_print_handler([](const std::string& msg) {
		spdlog::info(msg);
	});

	ap->set_print_json_handler([](const std::list<APClient::TextNode>& msg) {
		
		// make sure we dont show a message
		// after the fatal error message
		if (Core->fatalError) {
			return;
		}

		auto message = ap->render_json(msg, APClient::RenderFormat::TEXT);
		spdlog::info(message);
	});

	ap->set_bounced_handler([](const json& cmd) {
		if (GameHooks->isDeathLink) {
			auto tagsIt = cmd.find("tags");
			auto dataIt = cmd.find("data");

			if (tagsIt != cmd.end() && tagsIt->is_array() && std::find(tagsIt->begin(), tagsIt->end(), "DeathLink") != tagsIt->end()) {
				if (dataIt != cmd.end() && dataIt->is_object()) {
					json data = *dataIt;

					std::string source = data["source"].is_string() ? data["source"].get<std::string>() : "";
					std::string cause = data["cause"].is_string() ? data["cause"].get<std::string>() : "";

					if (!source.empty() && source != Core->pSlotName) {
						spdlog::info("DeathLink source: {} caused by: {}", source, cause);
						if (GameHooks->killPlayer()) {
							Core->diedByDeathLink = true;
						}
					}
				}
			}
		}
	});

	return true;
}


VOID CArchipelago::say(std::string message) {
	if (ap && ap->get_state() == APClient::State::SLOT_CONNECTED) {
		ap->Say(message);
	}
}


BOOLEAN CArchipelago::isConnected() {
	return ap && ap->get_state() == APClient::State::SLOT_CONNECTED;
}

VOID CArchipelago::update() {
	if (ap) ap->poll();

	if (isConnected()) {
		handleLocationChecks();
	}
}

VOID CArchipelago::handleLocationChecks() {
	std::list<int64_t> locations = GameHooks->checkedLocations;
	if (!isConnected() || locations.size() == 0) return;
	
	// if we are awarded the sould of nashandra then send game completion
	auto it = std::find(locations.begin(), locations.end(), 627000);
	if (it != locations.end()) {
		gameFinished();
		GameHooks->checkedLocations.clear();
		return;
	}

	if (ap->LocationChecks(locations)) {
		GameHooks->checkedLocations.clear();
	};
}

VOID CArchipelago::gameFinished() {
	if (ap) ap->StatusUpdate(APClient::ClientStatus::GOAL);
}

VOID CArchipelago::sendDeathLink() {
	if (!isConnected() || !GameHooks->isDeathLink) return;

	spdlog::info("Sending deathlink");

	json data{
		{"time", ap->get_server_time()},
		{"cause", "Dark Souls II."},
		{"source", ap->get_slot()},
	};
	ap->Bounce(data, {}, {}, { "DeathLink" });
}