#include <spdlog/spdlog.h>

#include "ArchipelagoInterface.h"
#include "apuuid.hpp"

#ifdef __EMSCRIPTEN__
#define DATAPACKAGE_CACHE "/settings/datapackage.json"
#define UUID_FILE "/settings/uuid"
#else
#define DATAPACKAGE_CACHE "datapackage.json" // TODO: place in %appdata%
#define UUID_FILE "uuid" // TODO: place in %appdata%
#endif

bool ap_sync_queued = false;
APClient* ap;

BOOL CArchipelago::Initialise(std::string URI) {
	spdlog::info("ClientArchipelago::Intialise");

	std::string uuid = ap_get_uuid(UUID_FILE);
	if (ap != nullptr) {
		ap->reset();
	}

	ap = new APClient(uuid, "Dark Souls III", URI);

	ap_sync_queued = false;
	ap->set_socket_connected_handler([]() {
		});

	ap->set_socket_disconnected_handler([]() {
		});

	ap->set_slot_connected_handler([](const json& data) {
		spdlog::info("Slot connected successfully, reading slot data ... ");
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
		std::list<std::string> tags;
		ap->ConnectSlot("SomeoneRandom", "", 5, tags, { 0, 4, 9});
		//if (GameHook->dIsDeathLink) { tags.push_back("DeathLink"); }
		//ap->ConnectSlot(Core->pSlotName, Core->pPassword, 5, tags, { 0,4,2 });
		});

	ap->set_items_received_handler([](const std::list<APClient::NetworkItem>& items) {

		if (!ap->is_data_package_valid()) {
			// NOTE: this should not happen since we ask for data package before connecting
			if (!ap_sync_queued) ap->Sync();
			ap_sync_queued = true;
			return;
		}
		});

	//TODO :   * you can still use `set_data_package` or `set_data_package_from_file` during migration to make use of the old cache

	ap->set_print_handler([](const std::string& msg) {
		spdlog::info(msg);
		});

	ap->set_print_json_handler([](const std::list<APClient::TextNode>& msg) {
		auto message = ap->render_json(msg, APClient::RenderFormat::TEXT);
		spdlog::info(message);
		});

	ap->set_bounced_handler([](const json& cmd) {
		spdlog::debug("Bad deathlink packet!");
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
}

VOID CArchipelago::gameFinished() {
	if (ap) ap->StatusUpdate(APClient::ClientStatus::GOAL);
}