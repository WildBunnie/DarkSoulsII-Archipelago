#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdint.h>

#define LIBDS2_IMPLEMENTATION
#include "ds2.h"
#include "overlay.h"

#include "apclient.hpp"
#include "apuuid.hpp"
#include "nlohmann/json.hpp"
#include "MinHook.h"
#include "kiero.h"
#include "imgui.h"
#include "imgui_impl_win32.h"

#if DS2_64
    #include <d3d11.h>
    #include "imgui_impl_dx11.h"
#elif DS2_32
    #include <d3d9.h>
    #include "imgui_impl_dx9.h"
#endif

#define LOG_PRINT(level, fmt, ...)                                         \
    do {                                                                   \
        SYSTEMTIME _st;                                                    \
        GetLocalTime(&_st);                                                \
        printf("[%02d-%02d-%04d %02d:%02d:%02d.%03d] [%s] (%s) " fmt "\n", \
            _st.wDay, _st.wMonth, _st.wYear,                               \
            _st.wHour, _st.wMinute, _st.wSecond,                           \
            _st.wMilliseconds,                                             \
            level,                                                         \
            __func__,                                                      \
            ##__VA_ARGS__);                                                \
    } while (0)

#define DEBUG_PRINT(fmt, ...) LOG_PRINT("DEBUG", fmt, ##__VA_ARGS__)
#define ERROR_PRINT(fmt, ...) LOG_PRINT("ERROR", fmt, ##__VA_ARGS__)

#define PANIC(fmt, ...) do { \
    char buf[1024]; \
    snprintf(buf, sizeof(buf), fmt, __VA_ARGS__); \
    MessageBoxA(NULL, buf, "Dark Souls II Archipelago Error", MB_OK | MB_ICONERROR); \
    ExitProcess(1); \
} while(0)

#define HOOK_LIST(X) \
    X(set_map_entity_picked_up,    DS2_FUNCTION_SET_MAP_ENTITY_PICKED_UP) \
    X(give_items_on_reward,        DS2_FUNCTION_GIVE_ITEMS_ON_REWARD) \
    X(give_items_on_shop_purchase, DS2_FUNCTION_GIVE_ITEMS_ON_SHOP_PURCHASE) \
    X(add_item_to_inventory,       DS2_FUNCTION_ADD_ITEM_TO_INVENTORY) \
    X(remove_item_from_inventory,  DS2_FUNCTION_REMOVE_ITEM_FROM_INVENTORY) \
    X(get_fmg_entry,               DS2_FUNCTION_GET_FMG_ENTRY) \
    X(get_hovering_item_info,      DS2_FUNCTION_GET_HOVERING_ITEM_INFO) \
    X(load_region_event_flags,     DS2_FUNCTION_LOAD_REGION_EVENT_FLAGS) \
    X(save_game,                   DS2_FUNCTION_SAVE_GAME) \
    X(write_event_flag,            DS2_FUNCTION_WRITE_EVENT_FLAG) \

// initialize global function pointers to the original functions
// these will be populated by minhook when creating a hook
#define INIT_ORIGINAL(name, addr) ds2_##name##_t original_##name = 0;
HOOK_LIST(INIT_ORIGINAL)
#undef INIT_ORIGINAL

// This needs to fit all our items that are in the item pool
// which would be stored here all at the same time if the user did a collect
// The locations file has like 3k lines (and num items = num locations)
// with most of them commented so this should be enough
// Still make it go arround cause of the checks
#define QUEUE_MAX 4096
typedef struct {
    uint32_t entries[QUEUE_MAX];
    int read;
    int write;
} Queue;

int queue_is_empty(Queue* q)
{
    return q->read == q->write;
}

int queue_push(Queue* q, uint32_t value)
{
    int next = (q->write + 1) % QUEUE_MAX;

    if (next == q->read) {
        return 0; // full
    }

    q->entries[q->write] = value;
    q->write = next;
    return 1;
}

int queue_pop(Queue* q, uint32_t* out)
{
    if (q->read == q->write) {
        return 0; // no more to read
    }

    *out = q->entries[q->read];
    q->read = (q->read + 1) % QUEUE_MAX;
    return 1;
}

// NOTE maybe this should be in ds2.h
const uint32_t UNUSED_ITEM_IDS[] = {60155010, 60155020, 60155030, 65240000, 65250000, 65260000, 65270000, 65280000, 65290000, 900008182, 900008183};
#define UNUSED_ITEM_COUNT (sizeof(UNUSED_ITEM_IDS) / sizeof(UNUSED_ITEM_IDS[0]))
#define UNUSED_SHOP_ITEM_ID 60375000

#define MAX_LOCATIONS 4096
#define MAX_ITEMS 2048
#define MAX_ITEM_NAME 128
#define MAX_LOCATION_REWARDS 16

// There are overlapping ids in different parameter tables
// so we add this value to the parameter id to make it unique
typedef enum {
    LOC_ITEMLOT_CHR   = 100'000'000,
    LOC_ITEMLOT_OTHER = 200'000'000,
    LOC_SHOP_LINEUP   = 300'000'000
} APLocationType;

typedef struct {
    int64_t archipelago_id;
    uint32_t item_id;
    uint32_t ap_item_id;
    wchar_t item_name[MAX_ITEM_NAME];
} LocationReward;

typedef struct {
    uint32_t location_key;
    APLocationType location_type;

    float shop_price;
    int keep_unrandomized;

    int64_t archipelago_ids[MAX_LOCATION_REWARDS];

    LocationReward location_rewards[MAX_LOCATION_REWARDS];
    int reward_count;
} APLocationMapping;

enum APItemType {
    APITEM_ITEM  = 1,
    APITEM_EVENT = 2
};

typedef struct {
    uint32_t item_id;
    APItemType item_type;
    int is_bundle;
    int reinforcement;

    wchar_t item_name[MAX_ITEM_NAME];
} APItemMapping;

typedef struct {
    uint32_t item_id;
    wchar_t item_name[MAX_ITEM_NAME];
} UnusedItem;

#define MAX_EVENT_FLAGS 128

typedef struct {
    int64_t item_received_count;

    int event_flag_count;
    uint32_t event_flags[MAX_EVENT_FLAGS];
} ModSaveData;

typedef struct {
    float x_coord;
    float y_coord;
    float z_coord;
} DS2PlayerPosition;

// The max slot name for ap is 16
// we make it 17 cause of the null terminator
#define MAX_SLOT_NAME 17
#define MAX_PASSWORD 256
#define MAX_SEED_SIZE 256
#define MAX_SERVER_URI 256
#define MAX_REFUSE_REASON 512
#define MAX_MAP_ID_SIZE 13
typedef struct {
    int stop;

    APClient* ap;

    int slot_refused;
    char slot_refuse_reason[MAX_REFUSE_REASON];

    char server_uri[MAX_SERVER_URI];
    char slot_name[MAX_SLOT_NAME];
    char password[MAX_PASSWORD];

    char seed[MAX_SEED_SIZE];
    int slot_data_loaded;

    ModSaveData save_data;
    int save_loaded;

    int goaled;

    Queue item_queue;
    Queue location_queue;
    CRITICAL_SECTION queue_lock;

    APLocationMapping location_mappings[MAX_LOCATIONS];
    int location_mapping_count;

    APItemMapping item_mappings[MAX_ITEMS];
    int item_mapping_count;

    UnusedItem unused_shop_item;
    UnusedItem unused_items[UNUSED_ITEM_COUNT];
    int unused_item_count;

    char map_id[MAX_MAP_ID_SIZE];
    DS2PlayerPosition* player_position_address;

    // TODO maybe cap the size
    std::vector<std::string> console_log;
} ModState;

static ModState state = {0};

APLocationMapping* get_location_mapping(uint32_t location_key)
{
    for (int i = 0; i < state.location_mapping_count; ++i) {
        if (state.location_mappings[i].location_key == location_key) {
            return &state.location_mappings[i];
        }
    }
    return 0;
}

APLocationMapping* get_location_mapping_by_id(int64_t location_id)
{
    for (int i = 0; i < state.location_mapping_count; ++i) {
        APLocationMapping* mapping = &state.location_mappings[i];
        for (int j = 0; j < MAX_LOCATION_REWARDS; ++j) {
            if (mapping->archipelago_ids[j] == location_id) {
                return mapping;
            }
        }
    }
    return 0;
}

APItemMapping* get_item_mapping(uint32_t item_id)
{
    for (int i = 0; i < state.item_mapping_count; ++i) {
        if (state.item_mappings[i].item_id == item_id) {
            return &state.item_mappings[i];
        }
    }
    return 0;
}

UnusedItem* get_unused_item(uint32_t item_id)
{
    if (item_id == UNUSED_SHOP_ITEM_ID){
        return &state.unused_shop_item;
    }
    for (int i = 0; i < state.unused_item_count; ++i) {
        if (state.unused_items[i].item_id == item_id) {
            return &state.unused_items[i];
        }
    }
    return 0;
}

void build_save_path(char* out_path)
{
    char dir1[] = "archipelago";
    char dir2[] = "\\save_data\\";
    char ext[]  = ".dat";

    size_t pos = 0;

    for (size_t i = 0; dir1[i] && pos < MAX_PATH - 1; ++i)
        out_path[pos++] = dir1[i];

    for (size_t i = 0; dir2[i] && pos < MAX_PATH - 1; ++i)
        out_path[pos++] = dir2[i];

    for (size_t i = 0; state.seed[i] && pos < MAX_PATH - 1; ++i)
        out_path[pos++] = state.seed[i];

    for (size_t i = 0; ext[i] && pos < MAX_PATH - 1; ++i)
        out_path[pos++] = ext[i];

    out_path[pos] = '\0';
}


void load_save_data()
{
    if (state.save_loaded) return;
    state.save_loaded = 1;
    DEBUG_PRINT("loading save data");

    char path[MAX_PATH];
    build_save_path(path);

    HANDLE file = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        DEBUG_PRINT("No save file found, starting fresh");
        return;
    }

    DWORD read;
    ReadFile(
        file,
        &state.save_data,
        sizeof(ModSaveData),
        &read,
        NULL
    );

    CloseHandle(file);
}


void store_save_data()
{
    if (!state.save_loaded) return;
    DEBUG_PRINT("saving save data");

    char path[MAX_PATH];
    build_save_path(path);

    HANDLE file = CreateFileA(
        path,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        DEBUG_PRINT("Failed to open save file for writing");
        return;
    }

    DWORD written;
    WriteFile(
        file,
        &state.save_data,
        sizeof(ModSaveData),
        &written,
        NULL
    );

    CloseHandle(file);
}

void handle_set_event_flag(uint32_t flag_id)
{
    DEBUG_PRINT("handling new set event flag: %d", flag_id);
    // check if already exists
    for (int i = 0; i < state.save_data.event_flag_count; ++i) {
        if (state.save_data.event_flags[i] == flag_id) return;
    }

    libds2_set_event_flag(flag_id, 1);
    state.save_data.event_flags[state.save_data.event_flag_count++] = flag_id;
}

void check_location(uint32_t id, APLocationType type)
{
    uint32_t location_key = id + (uint32_t)type;
    EnterCriticalSection(&state.queue_lock);
    queue_push(&state.location_queue, location_key);
    DEBUG_PRINT("pushed check %u", location_key);
    LeaveCriticalSection(&state.queue_lock);

    APLocationMapping* location_mapping = get_location_mapping(location_key);
    if (!location_mapping) return;

    for (int i = 0; i < location_mapping->reward_count; ++i) {
        LocationReward* reward = &location_mapping->location_rewards[i];
        if (reward->item_name[0] == L'\0') continue;

        UnusedItem* unused_item = get_unused_item(reward->item_id);
        if (!unused_item) continue;
        for (int j = 0; j < MAX_ITEM_NAME; j++) {
            unused_item->item_name[j] = reward->item_name[j];
            if (reward->item_name[j] == L'\0') break;
        }

        APItemMapping* item_mapping = get_item_mapping(reward->ap_item_id);
        if (item_mapping && item_mapping->item_type == APITEM_EVENT) {
            handle_set_event_flag(reward->ap_item_id);
        }
    }
}

#if DS2_64
void detour_set_map_entity_picked_up(DS2MapItemPackEntityData* p1, DS2MapItemPackEntityData* p2)
#elif DS2_32
void __fastcall detour_set_map_entity_picked_up(DS2MapItemPackEntityData* p1, void*, DS2MapItemPackEntityData* p2)
#endif
{
    DEBUG_PRINT("picked up %u", p1->item_lot_id);
    if (p1->item_lot_category == DS2_ItemLotParam2_Chr) {
        check_location(p1->item_lot_id, LOC_ITEMLOT_CHR);
    }
    else if (p1->item_lot_category == DS2_ItemLotParam2_Other) {
        check_location(p1->item_lot_id, LOC_ITEMLOT_OTHER);
    }
    original_set_map_entity_picked_up(p1, p2);
}

#if DS2_64
uint8_t detour_give_items_on_reward(uintptr_t p1, DS2ItemLotReward* p2, int32_t p3, int8_t p4, DS2ItemLotCategory p5)
#elif DS2_32
uint8_t __fastcall detour_give_items_on_reward(uintptr_t p1, void*, DS2ItemLotReward* p2, int32_t p3, int8_t p4, DS2ItemLotCategory p5)
#endif
{
    DEBUG_PRINT("was awarded %u", p2->item_lot_id);
    check_location(p2->item_lot_id, LOC_ITEMLOT_OTHER);
    return original_give_items_on_reward(p1, p2, p3, p4, p5);
}

#if DS2_64
uintptr_t detour_give_items_on_shop_purchase(uintptr_t p1, DS2BoughtShopItem* p2, uint16_t p3)
#elif DS2_32
uintptr_t __fastcall detour_give_items_on_shop_purchase(uintptr_t p1, void*, DS2BoughtShopItem* p2, uint16_t p3)
#endif
{
    DEBUG_PRINT("bought %u", p2->shop_lineup_id);
    check_location(p2->shop_lineup_id, LOC_SHOP_LINEUP);
    return original_give_items_on_shop_purchase(p1, p2, p3);
}

#if DS2_64
void detour_add_item_to_inventory(uintptr_t param_1, DS2ItemStruct* item)
#elif DS2_32
void __fastcall detour_add_item_to_inventory(uintptr_t param_1, void*, DS2ItemStruct* item)
#endif
{
    DEBUG_PRINT("item added to inventory: %d", item->item_id);

    if (item->item_id == 50900000) {   // giants kinship
        handle_set_event_flag(100972); // unlock nahandra
    }
    else if (item->item_id == 53600000) {  // eye of the priestess
        handle_set_event_flag(537000012); // see invisible things
    }

    return original_add_item_to_inventory(param_1, item);
}

#if DS2_64
uint32_t detour_remove_item_from_inventory(uintptr_t param_1, uintptr_t param_2, uintptr_t item, uint32_t quantity)
#elif DS2_32
uint32_t __fastcall detour_remove_item_from_inventory(uintptr_t param_1, void*, uintptr_t param_2, uintptr_t item, uint32_t quantity)
#endif
{
    uint32_t item_id = *(uint32_t*)(item + DS2_OFFSET(0x18, 0xC));

    if (item_id == 60536000) {
        DEBUG_PRINT("used pharros lockstone");
        return 0;
    }

    DEBUG_PRINT("removed item: %d", item_id);
    return original_remove_item_from_inventory(param_1, param_2, item, quantity);
}

void remove_special_characters(const wchar_t* input, wchar_t* output, int max_len)
{
    int j = 0;

    for (int i = 0; input[i] != L'\0' && j < max_len - 1; i++) {
        wchar_t ch = input[i];

        int is_alphanumeric =
            (ch >= L'0' && ch <= L'9') ||
            (ch >= L'A' && ch <= L'Z') ||
            (ch >= L'a' && ch <= L'z');

        if (is_alphanumeric ||
            ch == L'-' || ch == L'\'' || ch == L',' ||
            ch == L' ' || ch == L':' || ch == L'+' || ch == L'&')
        {
            output[j++] = ch;
        }
    }

    output[j] = L'\0';
}

const wchar_t* __cdecl detour_get_fmg_entry(DS2FMGFileID file_id, int32_t entry_id)
{
    static wchar_t cleaned_name[MAX_ITEM_NAME];

    if (file_id == DS2_FMG_ITEM_NAMES) {
        UnusedItem* unused_item = get_unused_item(entry_id);
        if (unused_item && unused_item->item_name[0] != L'\0') {

            remove_special_characters(
                unused_item->item_name,
                cleaned_name,
                MAX_ITEM_NAME
            );

            return cleaned_name;
        }
    }

    return original_get_fmg_entry(file_id, entry_id);
}

#if DS2_64
uintptr_t detour_get_hovering_item_info(uintptr_t p1, uintptr_t p2)
#elif DS2_32
uintptr_t __fastcall detour_get_hovering_item_info(uintptr_t p1, void*, uintptr_t p2)
#endif
{
    int offset = DS2_OFFSET(0x10, 0x8);
    uintptr_t ptr = *(uintptr_t*)(p1 + offset);
    if (ptr != 0) {
        uint32_t param_id = *(uintptr_t*)(ptr + offset);
        uint32_t location_key = param_id + (uint32_t)LOC_SHOP_LINEUP; // we only care for shops here

        APLocationMapping* mapping = get_location_mapping(location_key);
        if (mapping) {
            LocationReward* reward = &mapping->location_rewards[0];
            if (reward->item_id == UNUSED_SHOP_ITEM_ID) {
                for (int i = 0; i < MAX_ITEM_NAME; i++) {
                    state.unused_shop_item.item_name[i] = reward->item_name[i];
                    if (reward->item_name[i] == L'\0') break;
                }
            }
        }
    }
    return original_get_hovering_item_info(p1, p2);
}

#if DS2_64
void detour_load_region_event_flags(DS2EventFlagManager* manager, uint32_t region_id)
#elif DS2_32
void __fastcall detour_load_region_event_flags(DS2EventFlagManager* manager, void*, uint32_t region_id)
#endif
{
    original_load_region_event_flags(manager, region_id);

    for (int i = 0; i < state.save_data.event_flag_count; ++i) {
        libds2_set_event_flag(state.save_data.event_flags[i], 1);
    }
}

#if DS2_64
uint8_t detour_save_game(uintptr_t param_1, uint8_t param_2, uint8_t param_3)
#elif DS2_32
uint8_t __fastcall detour_save_game(uintptr_t param_1, void*, uint8_t param_2, uint8_t param_3)
#endif
{
    uint8_t result = original_save_game(param_1, param_2, param_3);
    if (state.save_loaded) {
        store_save_data();
    }
    return result;
}

#if DS2_64
uint8_t detour_write_event_flag(DS2EventFlagManager* manager, uint32_t flag_id, uint8_t value)
#elif DS2_32
uint8_t __fastcall detour_write_event_flag(DS2EventFlagManager* manager, void*, uint32_t flag_id, uint8_t value)
#endif
{
    uint8_t result = original_write_event_flag(manager, flag_id, value);
    if (result) {
        DEBUG_PRINT("setting event flag %u to %u", flag_id, value);
    }

    // TODO move this to apworld
    if (flag_id == 100973 && value == 1) { // killed nashandra
        state.goaled = 1;
    }

    return result;
}

#define MAX_ITEMLOT_PARAMS 2048
typedef struct {
    APLocationType location_type;

    int32_t guaranteed[MAX_ITEMLOT_PARAMS];
    int guaranteed_count;

    int32_t random[MAX_ITEMLOT_PARAMS];
    int random_count;
} ItemlotPatchContext;

void shuffle(void* ptr, int count, size_t element_size) {
    uint8_t* arr = (uint8_t*)ptr;

    for (int i = count - 1; i > 0; --i) {
        int j = rand() % (i + 1);

        for (size_t k = 0; k < element_size; ++k) {
            uint8_t tmp = arr[i * element_size + k];
            arr[i * element_size + k] = arr[j * element_size + k];
            arr[j * element_size + k] = tmp;
        }
    }
}

void itemlot_prepass_fn(ParamRowData* row_data, void* context) {
    DS2ItemLotParam2* param = (DS2ItemLotParam2*)row_data->data;
    ItemlotPatchContext* ctx = (ItemlotPatchContext*)context;

    uint32_t location_key = row_data->row->param_id + (uint32_t)ctx->location_type;
    if (get_location_mapping(location_key)) return; // leave these for the next step

    float chance_sum = 0.0f;
    float* chances = &param->chance_lot_0;
    for (int i = 0; i < 10; ++i) {
        chance_sum += chances[i];
    }

    if (chance_sum >= 100.0f) {
        ctx->guaranteed[ctx->guaranteed_count++] = row_data->row->reward_offset;
    }
    else if (chance_sum > 0) {
        ctx->random[ctx->random_count++] = row_data->row->reward_offset;
    }
}

void itemlot_patch_fn(ParamRowData* row_data, void* context) {
    DS2ItemLotParam2* param = (DS2ItemLotParam2*)row_data->data;
    ItemlotPatchContext* ctx = (ItemlotPatchContext*)context;

    uint32_t location_key = row_data->row->param_id + (uint32_t)ctx->location_type;
    APLocationMapping* location_mapping = get_location_mapping(location_key);
    if (location_mapping && location_mapping->keep_unrandomized) return;

    int32_t* items = &param->item_lot_0;
    float* chances = &param->chance_lot_0;
    uint8_t* amounts = &param->amount_lot_0;
    uint8_t* reinforcements = &param->reinforcement_lot_0;
    uint8_t* infusions = &param->infusion_lot_0;

    if (location_mapping) {
        for (int i = 0; i < 10; ++i) chances[i] = 0.0f; // zero out just in case

        for (int i = 0; i < location_mapping->reward_count; ++i) {
            LocationReward* reward = &location_mapping->location_rewards[i];

            APItemMapping* item_mapping = get_item_mapping(reward->item_id);
            if (item_mapping && item_mapping->is_bundle) {
            	amounts[i] = reward->item_id % 1000;
                items[i] = reward->item_id - amounts[i];
            }
            else {
                items[i] = reward->item_id;
                amounts[i] = 1;
            }

            if (item_mapping) {
                reinforcements[i] = item_mapping->reinforcement;
            }

            infusions[i] = 0;
            chances[i] = 100.0f;
        }
    }
    else {
        float chance_sum = 0.0f;
        for (int i = 0; i < 10; ++i) chance_sum += chances[i];

        if (chance_sum >= 100.0f) {
            row_data->row->reward_offset = ctx->guaranteed[ctx->guaranteed_count--];
        }
        else if (chance_sum > 0) {
            row_data->row->reward_offset = ctx->random[ctx->random_count--];
        }
    }
}

typedef struct {
    int32_t item_id;
    int32_t item_price;
} ItemWithPrice;

typedef struct {
    int32_t item_id;
    uint8_t quantity;
} ShopItemData;

#define MAX_SHOP_PARAMS 2048
#define MAX_ITEMS 2048
typedef struct {
    ItemWithPrice item_prices[MAX_ITEMS];
    int item_price_count;

    ShopItemData unique[MAX_SHOP_PARAMS];
    int unique_count;

    ShopItemData infinite[MAX_SHOP_PARAMS];
    int infinite_count;

    ShopItemData non_unique[MAX_SHOP_PARAMS];
    int non_unique_count;
} ShopPatchContext;

void price_patch_fn(ParamRowData* row_data, void* context) {
    DS2ItemParam* param = (DS2ItemParam*)row_data->data;
    ShopPatchContext* ctx = (ShopPatchContext*)context;

    ctx->item_prices[ctx->item_price_count].item_id = row_data->row->param_id;
    ctx->item_prices[ctx->item_price_count++].item_price = param->base_price;

    param->base_price = 1;
}

float calculate_price_rate(float current_price_rate, int32_t item_id, ShopPatchContext* context)
{
    for (int i = 0; i < context->item_price_count; ++i) {
        ItemWithPrice* price_data = &context->item_prices[i];
        if (price_data->item_id == item_id){
            return current_price_rate * (float)price_data->item_price;
        }
    }
    return current_price_rate;
}

void shop_prepass_fn(ParamRowData* row_data, void* context) {
    DS2ShopLineupParam* param = (DS2ShopLineupParam*)row_data->data;
    ShopPatchContext* ctx = (ShopPatchContext*)context;

    uint32_t location_key = row_data->row->param_id + (uint32_t)LOC_SHOP_LINEUP;
    if (get_location_mapping(location_key)) return; // leave these for the next step

    ShopItemData data;
    data.item_id = param->item_id;
    data.quantity = param->quantity;

    if (param->quantity == 1) {
        ctx->unique[ctx->unique_count++] = data;
    }
    else if (param->quantity == 255) {
        ctx->infinite[ctx->infinite_count++] = data;
    }
    else {
        ctx->non_unique[ctx->non_unique_count++] = data;
    }
}

void shop_patch_fn(ParamRowData* row_data, void* context) {
    DS2ShopLineupParam* param = (DS2ShopLineupParam*)row_data->data;
    ShopPatchContext* ctx = (ShopPatchContext*)context;

    uint32_t location_key = row_data->row->param_id + (uint32_t)LOC_SHOP_LINEUP;
    param->price_rate = calculate_price_rate(param->price_rate, param->item_id, ctx);

    APLocationMapping* location_mapping = get_location_mapping(location_key);
    if (location_mapping && location_mapping->keep_unrandomized) return;

    if (location_mapping) {
        LocationReward* reward = &location_mapping->location_rewards[0];
        param->item_id = reward->item_id;
        param->disable_flag = -1;
        param->quantity = 1;
        param->price_rate = location_mapping->shop_price;
    }
    else {
        ShopItemData data;

        if (param->quantity == 1) {
            data = ctx->unique[--ctx->unique_count];
        }
        else if (param->quantity == 255) {
            data = ctx->infinite[--ctx->infinite_count];
        }
        else {
            data = ctx->non_unique[--ctx->non_unique_count];
        }

        // calculate price based on the original item at this place
        param->item_id = data.item_id;
        param->quantity = data.quantity;
    }
}

void randomize()
{
    unsigned int hash_seed = 0;
    for (int i = 0; state.seed[i] != '\0'; ++i) {
        hash_seed = hash_seed * 31 + (unsigned char)state.seed[i];
    }
    srand(hash_seed);

    ItemlotPatchContext icontext = {0};
    icontext.location_type = LOC_ITEMLOT_OTHER;
    libds2_patch_param_table(DS2_PARAM_ITEM_LOT_PARAM2_OTHER, itemlot_prepass_fn, &icontext);
    shuffle(icontext.guaranteed, icontext.guaranteed_count, sizeof(int32_t));
    shuffle(icontext.random, icontext.random_count, sizeof(int32_t));
    libds2_patch_param_table(DS2_PARAM_ITEM_LOT_PARAM2_OTHER, itemlot_patch_fn, &icontext);

    icontext = {0};
    icontext.location_type = LOC_ITEMLOT_CHR;
    libds2_patch_param_table(DS2_PARAM_ITEM_LOT_PARAM2_CHR, itemlot_prepass_fn, &icontext);
    shuffle(icontext.guaranteed, icontext.guaranteed_count, sizeof(int32_t));
    shuffle(icontext.random, icontext.random_count, sizeof(int32_t));
    libds2_patch_param_table(DS2_PARAM_ITEM_LOT_PARAM2_CHR, itemlot_patch_fn, &icontext);

    ShopPatchContext scontext = {0};
    libds2_patch_param_table(DS2_PARAM_ITEM_PARAM, price_patch_fn, &scontext);

    libds2_patch_param_table(DS2_PARAM_SHOP_LINEUP_PARAM, shop_prepass_fn, &scontext);
    shuffle(scontext.unique, scontext.unique_count, sizeof(ShopItemData));
    shuffle(scontext.infinite, scontext.infinite_count, sizeof(ShopItemData));
    shuffle(scontext.non_unique, scontext.non_unique_count, sizeof(ShopItemData));
    libds2_patch_param_table(DS2_PARAM_SHOP_LINEUP_PARAM, shop_patch_fn, &scontext);

    // TODO if player is ingame send him to last bonfire
}

typedef struct {
    int remove_weight;
} RequirementsPatchContext;

void remove_weapon_requirements(ParamRowData* row_data, void* context) {
    DS2WeaponParam* param = (DS2WeaponParam*)row_data->data;
    RequirementsPatchContext* ctx = (RequirementsPatchContext*)context;
    if (row_data->row->param_id < 1000000) return; // ignore npc weapons

    param->str_requirement = 0;
    param->dex_requirement = 0;
    param->int_requirement = 0;
    param->fth_requirement = 0;

    if (ctx && ctx->remove_weight) {
        param->weight = 0.0f;
    }
}

void remove_spell_requirements(ParamRowData* row_data, void* context) {
    DS2SpellParam* param = (DS2SpellParam*)row_data->data;
    RequirementsPatchContext* ctx = (RequirementsPatchContext*)context;

    param->int_requirement = 0;
    param->fth_requirement = 0;
}

void remove_armor_requirements(ParamRowData* row_data, void* context) {
    DS2ArmorParam* param = (DS2ArmorParam*)row_data->data;
    RequirementsPatchContext* ctx = (RequirementsPatchContext*)context;

    param->strength_requirement = 0;
    param->dexterity_requirement = 0;
    param->intelligence_requirement = 0;
    param->faith_requirement = 0;

    if (ctx && ctx->remove_weight) {
        param->weight = 0.0f;
    }
}

void remove_ring_weights(ParamRowData* row_data, void* context) {
    DS2RingParam* param = (DS2RingParam*)row_data->data;
    param->weight = 0.0f;
}

enum ItemsHandling {
    NO_ITEMS           = 0b000,
    OTHER_WORLDS       = 0b001,
    OUR_WORLD          = 0b010,
    STARTING_INVENTORY = 0b100
};

#if DS2_64
    #define GAME_VERSION 0
#elif DS2_32
    #define GAME_VERSION 1
#endif

void ap_on_room_info()
{
    int items_handling = OTHER_WORLDS | STARTING_INVENTORY;
    APClient::Version ap_version = { 0, 6, 4 };
    state.ap->ConnectSlot(state.slot_name, state.password, items_handling, {}, ap_version);
}

void ap_on_slot_connected(const nlohmann::json& data)
{
    // DEBUG_PRINT("%s\n", data.dump(4).c_str());
    // TODO handle slot data not matching

    if (state.slot_data_loaded) return;

	if (data.contains("game_version") && data.at("game_version") != GAME_VERSION) {
        PANIC("The client's game version does not match the version chosen on the yaml file.\n"
        "Please change to the correct game version or change the yaml file and generate a new game.");
	}

    // get seed
    std::string temp =
        state.ap->get_player_alias(state.ap->get_player_number()) + "_" +
        std::to_string(state.ap->get_team_number()) + "_" +
        state.ap->get_seed();

    const char* player_seed = temp.c_str();

    int i;
    for (i = 0; player_seed[i] != '\0' && i < MAX_SEED_SIZE - 1; ++i) {
        state.seed[i] = player_seed[i];
    }
    state.seed[i] = '\0';

    load_save_data();

    // location mappings
    const nlohmann::json& locations = data.at("location_data");
    for (size_t i = 0; i < locations.size(); ++i) {
        const nlohmann::json& entry = locations[i];
        APLocationMapping* mapping = &state.location_mappings[state.location_mapping_count++];
        mapping->location_key = entry.at("location_key").get<int64_t>();

        if (mapping->location_key >= LOC_SHOP_LINEUP) {
            mapping->location_type = LOC_SHOP_LINEUP;
        }
        else if (mapping->location_key >= LOC_ITEMLOT_OTHER) {
            mapping->location_type = LOC_ITEMLOT_OTHER;
        }
        else if (mapping->location_key >= LOC_ITEMLOT_CHR) {
            mapping->location_type = LOC_ITEMLOT_CHR;
        }
        else {
            ERROR_PRINT("Invalid location key: %d", mapping->location_key);
        }

        if (entry.contains("shop_price")) {
            mapping->shop_price = entry.at("shop_price").get<float>();
        }

        if (entry.contains("keep_unrandomized") &&
            entry.at("keep_unrandomized").is_boolean() &&
            entry.at("keep_unrandomized").get<bool>())
        {
            mapping->keep_unrandomized = 1;
            continue;
        }

        const nlohmann::json& ids = entry.at("archipelago_ids");
        for (size_t j = 0; j < ids.size(); ++j) {
            mapping->archipelago_ids[j] = ids[j].get<int64_t>();
        }
    }

    // item mappings
    const nlohmann::json& items = data.at("item_data");
    for (size_t i = 0; i < items.size(); ++i) {
        const nlohmann::json& entry = items[i];

        APItemMapping* mapping = &state.item_mappings[state.item_mapping_count++];
        mapping->item_id = entry.at("item_id").get<int>();
        mapping->item_type = (APItemType)entry.at("item_type").get<int>();
        mapping->is_bundle = entry.at("is_bundle").get<int>();
        mapping->reinforcement = entry.at("reinforcement").get<int>();

        if (mapping->item_type != APITEM_ITEM) {
            std::string item_name = state.ap->get_item_name(
                mapping->item_id,
                state.ap->get_player_game(state.ap->get_player_number())
            );

            MultiByteToWideChar(
                CP_UTF8,
                0,
                item_name.c_str(),
                -1,
                mapping->item_name,
                MAX_ITEM_NAME
            );
        }
    }

    RequirementsPatchContext context = {0};
	if (data.contains("no_equip_load") && data.at("no_equip_load") == 1) {
	   context.remove_weight = 1;
	   libds2_patch_param_table(DS2_PARAM_RING_PARAM, remove_ring_weights, &context);
	}
	if (data.contains("no_weapon_req") && data.at("no_weapon_req") == 1) {
	   libds2_patch_param_table(DS2_PARAM_WEAPON_PARAM, remove_weapon_requirements, &context);
	}
	if (data.contains("no_spell_req") && data.at("no_spell_req") == 1) {
	   libds2_patch_param_table(DS2_PARAM_SPELL_PARAM, remove_spell_requirements, &context);
	}
	if (data.contains("no_armor_req") && data.at("no_armor_req") == 1) {
       libds2_patch_param_table(DS2_PARAM_ARMOR_PARAM, remove_armor_requirements, &context);
	}

    // scout all locations
    std::list<int64_t> locations_list;
    std::set<int64_t> missing_locations = state.ap->get_missing_locations();
    std::set<int64_t> checked_locations = state.ap->get_checked_locations();
    locations_list.insert(locations_list.end(), missing_locations.begin(), missing_locations.end());
    locations_list.insert(locations_list.end(), checked_locations.begin(), checked_locations.end());
    state.ap->LocationScouts(locations_list);

    state.slot_data_loaded = 1;
}

void ap_on_slot_refused(const std::list<std::string>& errors)
{
    state.slot_refuse_reason[0] = '\0';

    int offset = 0;
    int iteration = 1;
    for (const auto& error : errors) {
        const char* msg = error.c_str();

        while (*msg != '\0' && offset < MAX_REFUSE_REASON - 1) {
            state.slot_refuse_reason[offset++] = *msg++;
        }

        if (iteration < errors.size() && offset < MAX_REFUSE_REASON - 1) {
            state.slot_refuse_reason[offset++] = ',';
            state.slot_refuse_reason[offset++] = ' ';
        }

        iteration++;
    }

    state.slot_refuse_reason[offset] = '\0';
    state.slot_refused = 1;
}

void ap_on_location_info(const std::list<APClient::NetworkItem>& network_items)
{
    for (const auto& network_item : network_items) {
        APLocationMapping* location_mapping = get_location_mapping_by_id(network_item.location);

        LocationReward* reward = &location_mapping->location_rewards[location_mapping->reward_count++];

        uint32_t unused_item_id = 0;
        if (location_mapping->location_type == LOC_SHOP_LINEUP) { // aka is shop
            unused_item_id = UNUSED_SHOP_ITEM_ID;
        }
        else {
            unused_item_id = state.unused_items[location_mapping->reward_count - 1].item_id;
        }

        reward->archipelago_id = network_item.location;
        reward->ap_item_id = network_item.item;

        int is_our_item = network_item.player == state.ap->get_player_number();
        if (is_our_item) {
            APItemMapping* item_mapping = get_item_mapping(network_item.item);
            if (item_mapping->item_type == APITEM_ITEM) {
                reward->item_id = network_item.item;
            }
            else {
                reward->item_id = unused_item_id;

                std::string item_name = state.ap->get_item_name(
                    network_item.item,
                    state.ap->get_player_game(network_item.player)
                );

                MultiByteToWideChar(
                    CP_UTF8,
                    0,
                    item_name.c_str(),
                    -1,
                    reward->item_name,
                    MAX_ITEM_NAME
                );
            }
        }
        else {
            reward->item_id = unused_item_id;

            std::string player_name = state.ap->get_player_alias(network_item.player);
            std::string item_name = state.ap->get_item_name(
                network_item.item,
                state.ap->get_player_game(network_item.player)
            );
            std::string full_name = player_name + "'s " + item_name;

            MultiByteToWideChar(
                CP_UTF8,
                0,
                full_name.c_str(),
                -1,
                reward->item_name,
                MAX_ITEM_NAME
            );
        }
    }

    randomize();
}

void ap_on_items_received(const std::list<APClient::NetworkItem>& received_items)
{
    for (const auto& item : received_items) {
        if (item.index + 1 > state.save_data.item_received_count) {
            queue_push(&state.item_queue, (uint32_t)item.item);
            DEBUG_PRINT("pushed item %u", (uint32_t)item.item);
            state.save_data.item_received_count++;
        }
    }
}

void ap_on_print(const std::string& msg) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char buffer[512];
    snprintf(buffer, sizeof(buffer),
        "[%02d:%02d:%02d] %s",
        st.wHour, st.wMinute, st.wSecond,
        msg.c_str()
    );

    DEBUG_PRINT("%s", msg.c_str());

    state.console_log.push_back(buffer);
}

void ap_on_print_json(const std::list<APClient::TextNode>& msg) {
    std::string message = state.ap->render_json(msg, APClient::RenderFormat::TEXT);
    ap_on_print(message);
}

APClient* setup_apclient()
{
    if (state.server_uri[0] == '\0') {
        return 0;
    }

    APClient* client = new APClient(
        ap_get_uuid("archipelago/archipelago_uuid"),
        "Dark Souls II",
        state.server_uri
    );

    client->set_room_info_handler(ap_on_room_info);
    client->set_slot_connected_handler(ap_on_slot_connected);
    client->set_slot_refused_handler(ap_on_slot_refused);
    client->set_location_info_handler(ap_on_location_info);
    client->set_items_received_handler(ap_on_items_received);
    client->set_print_handler(ap_on_print);
    client->set_print_json_handler(ap_on_print_json);

    return client;
}

void handle_check_locations()
{
    if (queue_is_empty(&state.location_queue)) return;

    EnterCriticalSection(&state.queue_lock);

    uint32_t location_key;
    std::list<int64_t> ids_to_check;
    while (queue_pop(&state.location_queue, &location_key)) {
        DEBUG_PRINT("popped check %u", location_key);

        APLocationMapping* mapping = get_location_mapping(location_key);
        if (!mapping) continue;

        for (int i = 0; i < mapping->reward_count; ++i) {
            ids_to_check.push_back(mapping->location_rewards[i].archipelago_id);
        }
    }

    LeaveCriticalSection(&state.queue_lock);

    if (!ids_to_check.empty()) {
        state.ap->LocationChecks(ids_to_check);
    }
}

void handle_give_items()
{
    if (queue_is_empty(&state.item_queue)) return;
    if (libds2_get_player_state() != DS2_GAMESTATE_INGAME) return;
    if (libds2_is_item_popup_open()) return;

    uint32_t item_id;
    DS2ItemBagContent item_bag = {0};
    while (queue_pop(&state.item_queue, &item_id)) {
        DEBUG_PRINT("popped item %u", item_id);

        APItemMapping* item_mapping = get_item_mapping(item_id);
        if (!item_mapping) {
            ERROR_PRINT("item %d does not have a mapping", item_id);
            continue;
        }

        DS2ItemStruct item_struct = {0};
        item_struct.item_id = item_id;

        // this actually sets it to have the max amount of durability
        // if I do -1.0f it goes to -1 durability for some reason
        uint32_t value = 0xFFFFFFFF;
        item_struct.durability =  *(float*)&value;

        item_struct.amount = 1;
        item_struct.upgrade_level = (uint8_t)item_mapping->reinforcement;
        item_struct.infusion = 0;

        if (item_mapping->item_type == APITEM_ITEM) {
            if (item_mapping->is_bundle) {
            	item_struct.amount = item_struct.item_id % 1000;
                item_struct.item_id -= item_struct.amount;
            }
        }
        else if (item_mapping->item_type == APITEM_EVENT) {
            handle_set_event_flag(item_id);

            UnusedItem* actual_item = &state.unused_items[item_bag.item_count];
            item_struct.item_id = actual_item->item_id;

            // give the custom item the correct name
            for (int i = 0; i < MAX_ITEM_NAME; i++) {
                actual_item->item_name[i] = item_mapping->item_name[i];
                if (item_mapping->item_name[i] == L'\0') break;
            }
        }

        item_bag.items[item_bag.item_count++] = item_struct;
        if (item_bag.item_count == 8) {
            libds2_give_items(item_bag);
            item_bag = {0};
            break; // wait for popup
        }
    }

    if (item_bag.item_count > 0) {
        libds2_give_items(item_bag);
    }
}

constexpr std::string_view MAP_ID_THINGS_BETWIXT = "m10_02_00_00";
constexpr std::string_view MAP_ID_MAJULA = "m10_04_00_00";
constexpr std::string_view MAP_ID_FOREST_OF_FALLEN_GIANTS = "m10_10_00_00";
constexpr std::string_view MAP_ID_BRIGHTSTONE_COVE_TSELDORA = "m10_14_00_00";
constexpr std::string_view MAP_ID_ALDIAS_KEEP = "m10_15_00_00";
constexpr std::string_view MAP_ID_LOST_BASTILLE = "m10_16_00_00";
constexpr std::string_view MAP_ID_SINNERS_RISE = "m10_16_00_01"; // Artificial Map ID.
constexpr std::string_view MAP_ID_EARTHEN_PEAK = "m10_17_00_00";
constexpr std::string_view MAP_ID_HARVEST_VALLEY = "m10_17_00_01"; // Artificial Map ID.
constexpr std::string_view MAP_ID_NOMANS_WHARF = "m10_18_00_00";
constexpr std::string_view MAP_ID_IRON_KEEP = "m10_19_00_00";
constexpr std::string_view MAP_ID_HUNTSMANS_COPSE = "m10_23_00_00";
constexpr std::string_view MAP_ID_THE_GUTTER = "m10_25_00_00";
constexpr std::string_view MAP_ID_DRAGON_AERIE = "m10_27_00_00";
constexpr std::string_view MAP_ID_PATH_TO_THE_SHADED_WOODS = "m10_29_00_00"; // Treated as Majula by tracker.
constexpr std::string_view MAP_ID_UNSEEN_PATH_TO_HEIDES = "m10_30_00_00";
constexpr std::string_view MAP_ID_HEIDES_TOWER_OF_FLAME = "m10_31_00_00";
constexpr std::string_view MAP_ID_SHADED_WOODS = "m10_32_00_00";
constexpr std::string_view MAP_ID_DOORS_OF_PHARROS = "m10_33_00_00";
constexpr std::string_view MAP_ID_GRAVE_OF_SAINTS = "m10_34_00_00";
constexpr std::string_view MAP_ID_MEMORIES_OF_THE_GIANTS = "m20_10_00_00";
constexpr std::string_view MAP_ID_SHRINE_OF_AMANA = "m20_11_00_00";
constexpr std::string_view MAP_ID_DRANGLEIC_CASTLE = "m20_21_00_00";
constexpr std::string_view MAP_ID_UNDEAD_CRYPT = "m20_24_00_00";
constexpr std::string_view MAP_ID_DRAGON_MEMORIES = "m20_26_00_00";
constexpr std::string_view MAP_ID_CHAOS_OF_THE_ABYSS = "m40_03_00_00";
constexpr std::string_view MAP_ID_SHULVA = "m50_35_00_00";
constexpr std::string_view MAP_ID_DRAGONS_SANCTUM = "m50_35_00_01"; // Artificial Map ID.
constexpr std::string_view MAP_ID_DRAGONS_REST = "m50_35_00_02"; // Artificial Map ID.
constexpr std::string_view MAP_ID_CAVE_OF_THE_DEAD = "m50_35_00_03"; // Artificial Map ID.
constexpr std::string_view MAP_ID_BRUME_TOWER = "m50_36_00_00";
constexpr std::string_view MAP_ID_IRON_PASSAGE = "m50_36_00_01"; // Artificial Map ID.
constexpr std::string_view MAP_ID_MEMORY_OF_OLD_IRON_KING = "m50_36_00_02"; // Artificial Map ID.
constexpr std::string_view MAP_ID_ELEUM_LOYCE = "m50_37_00_00";
constexpr std::string_view MAP_ID_GRAND_CATHEDRAL = "m50_37_00_01"; // Artificial Map ID.
constexpr std::string_view MAP_ID_FRIGID_OUTSKIRTS = "m50_37_00_02"; // Artificial Map ID.
constexpr std::string_view MAP_ID_MEMORY_OF_THE_KING = "m50_38_00_00";

void send_map_id_changed()
{
    if (!state.ap || state.ap->get_state() != APClient::State::SLOT_CONNECTED) return;

    DEBUG_PRINT("sending MapUpdate bounce packet for map_id: %s", state.map_id);

    nlohmann::json data{
        {"type", "MapUpdate"},
        {"mapId", state.map_id},
    };
    bool success = state.ap->Bounce(data, {state.ap->get_game()}, {state.ap->get_player_number()}, {});
    if (!success) {
        DEBUG_PRINT("FAILED sending MapUpdate bounce packet for map_id: %s", state.map_id);
    }
}

void load_player_position()
{
    if (state.player_position_address) return;
    uintptr_t base_address = (uintptr_t)GetModuleHandle(0);
    state.player_position_address = (DS2PlayerPosition*)resolve_pointer_chain(base_address, player_position_chain, sizeof(player_position_chain) / sizeof(player_position_chain[0]));
}

void refine_map_id(std::string& new_map_id)
{
    load_player_position();
    if (!state.player_position_address) return;
    DS2PlayerPosition player = *state.player_position_address;

    // Use player position coordinates to alter the found Map ID to improve accuracy.
    if (new_map_id == MAP_ID_THINGS_BETWIXT) {
        if (player.x_coord > -90) {
            new_map_id = MAP_ID_MAJULA;
        }
    } else if (new_map_id == MAP_ID_MAJULA) {
        if (player.z_coord < -135 && state.map_id == MAP_ID_THINGS_BETWIXT) {
            // Prevent map tab stuttering at boundary with Things Betwixt.
            new_map_id = MAP_ID_THINGS_BETWIXT;
        } else if (player.x_coord < -150 && player.y_coord > 23.5) {
            new_map_id = MAP_ID_HUNTSMANS_COPSE;
        }
    } else if (new_map_id == MAP_ID_FOREST_OF_FALLEN_GIANTS) {
        if (player.x_coord > -55 && player.z_coord > -125) {
            new_map_id = MAP_ID_MAJULA;
        }
    } else if (new_map_id == MAP_ID_BRIGHTSTONE_COVE_TSELDORA) {
        if (player.y_coord < 132 && player.z_coord < -215) {
            new_map_id = MAP_ID_DOORS_OF_PHARROS;
        }
    } else if (new_map_id == MAP_ID_LOST_BASTILLE) {
        if (player.y_coord < -70 || player.z_coord > 645) {
            new_map_id = MAP_ID_SINNERS_RISE;
        }
    } else if (new_map_id == MAP_ID_EARTHEN_PEAK) {
        if (player.y_coord > 150) {
            new_map_id = MAP_ID_IRON_KEEP;
        } else if (player.x_coord > -483 && player.z_coord < 252) {
            new_map_id = MAP_ID_HUNTSMANS_COPSE;
        } else if (player.x_coord < -579 && player.y_coord < 55) {
            // Broken statue area which extends past the boundary line for Earthen Peak.
            new_map_id = MAP_ID_HARVEST_VALLEY;
        } else if (player.y_coord > 58 || player.z_coord > 469) {
            return; // Leave as Earthen Peak.
        } else {
            new_map_id = MAP_ID_HARVEST_VALLEY;
        }
    } else if (new_map_id == MAP_ID_NOMANS_WHARF) {
        if (player.z_coord < 440) {
            new_map_id = MAP_ID_HEIDES_TOWER_OF_FLAME;
        }
    } else if (new_map_id == MAP_ID_HUNTSMANS_COPSE) {
        if (player.x_coord > -150 && player.y_coord < 23.5) {
            new_map_id = MAP_ID_MAJULA;
        } else if (player.x_coord < -483 && player.z_coord > 252) {
            new_map_id = MAP_ID_HARVEST_VALLEY;
        }
    } else if (new_map_id == MAP_ID_THE_GUTTER) {
        if (player.y_coord > -120.5) {
            new_map_id = MAP_ID_MAJULA;
        }
    } else if (new_map_id == MAP_ID_PATH_TO_THE_SHADED_WOODS) {
        if (player.x_coord < -175) {
            new_map_id = MAP_ID_SHADED_WOODS;
        }
    } else if (new_map_id == MAP_ID_HEIDES_TOWER_OF_FLAME) {
        if (player.z_coord < 140) {
            new_map_id = MAP_ID_MAJULA;
        }
    } else if (new_map_id == MAP_ID_SHADED_WOODS) {
        if (player.y_coord > 95) {
            new_map_id = MAP_ID_DOORS_OF_PHARROS;
        } else if (player.z_coord < -336 || (player.x_coord > -253 && player.z_coord < -193)) {
            new_map_id = MAP_ID_DRANGLEIC_CASTLE;
        }
    } else if (new_map_id == MAP_ID_DOORS_OF_PHARROS) {
        if (player.y_coord < 95) {
            new_map_id = MAP_ID_SHADED_WOODS;
        } else if (player.y_coord > 132) {
            new_map_id = MAP_ID_BRIGHTSTONE_COVE_TSELDORA;
        }
    } else if (new_map_id == MAP_ID_GRAVE_OF_SAINTS) {
        if (player.x_coord < -60 && player.y_coord < -29 && player.y_coord > -38) {
            return; // Leave as Grave of Saints.
        } else if (player.y_coord < -120.5) {
            new_map_id = MAP_ID_THE_GUTTER;
        } else {
            new_map_id = MAP_ID_MAJULA;
        }
    } else if (new_map_id == MAP_ID_SHRINE_OF_AMANA) {
        if (player.y_coord < -70) {
            new_map_id = MAP_ID_UNDEAD_CRYPT;
        }
    } else if (new_map_id == MAP_ID_UNDEAD_CRYPT) {
        if (player.y_coord > -70) {
            new_map_id = MAP_ID_SHRINE_OF_AMANA;
        }
    } else if (new_map_id == MAP_ID_SHULVA) {
        if (player.y_coord < -69.3) {
            new_map_id = MAP_ID_DRAGONS_REST;
        } else if (player.y_coord < -5 || (player.x_coord < -10 && player.z_coord < -6)) {
            new_map_id = MAP_ID_DRAGONS_SANCTUM;
        } else if (player.x_coord < 9 && player.z_coord > 36.1) {
            if (player.x_coord > -1.1 && player.y_coord > 25.8) {
                // Pagan tree area, leave as Shulva.
                return;
            } else {
                new_map_id = MAP_ID_CAVE_OF_THE_DEAD;
            }
        } else if (player.x_coord < -18 && player.y_coord < 25 && player.y_coord > 16.4) {
            // Middle of elevator by Priestess' Chamber bonfire.
            new_map_id = MAP_ID_CAVE_OF_THE_DEAD;
        }
    } else if (new_map_id == MAP_ID_BRUME_TOWER) {
        if (player.x_coord < -155 && player.y_coord < -0.5 && player.y_coord > -27 && player.z_coord > 420) {
            new_map_id = MAP_ID_IRON_PASSAGE;
        } else if (player.z_coord > 585) {
            new_map_id = MAP_ID_MEMORY_OF_OLD_IRON_KING;
        } else if (player.z_coord > 465) {
            if (player.y_coord < 65 && player.y_coord > 25) {
                // Cursed side tower with Maldron the Assassin, leave as Brume Tower.
                return;
            } else if (player.y_coord < 71.4) {
                // Beginning of memory before first dropdown.
                new_map_id = MAP_ID_MEMORY_OF_OLD_IRON_KING;
            }
        }
    } else if (new_map_id == MAP_ID_ELEUM_LOYCE) {
        if (player.z_coord > 5) {
            new_map_id = MAP_ID_FRIGID_OUTSKIRTS;
        } else if (player.x_coord > 76 && player.z_coord > -80) {
            new_map_id = MAP_ID_GRAND_CATHEDRAL;
        }
    }
}

void handle_current_map_id()
{
    if (libds2_get_player_state() != DS2_GAMESTATE_INGAME) return;

    // Verify memory address points to a Map ID. This location can contain garbage data.
    uintptr_t base_address = (uintptr_t)GetModuleHandle(0);
    char* game_map_id = (char*)base_address + DS2_LOCATION_MAP_ID;
    // Note: didn't use regex to avoid adding an import, Map ID pattern is "m\d0_\d\d_00_00".
    if (game_map_id[0] != 'm' ||
        game_map_id[2] != '0' ||
        game_map_id[3] != '_' ||
        game_map_id[6] != '_' ||
        game_map_id[7] != '0' ||
        game_map_id[8] != '0' ||
        game_map_id[9] != '_' ||
        game_map_id[10] != '0' ||
        game_map_id[11] != '0') {
            return;
        }

    // Refine the Map ID using player coordinates to improve accuracy and fix map tab stuttering at boundaries.
    std::string new_map_id(game_map_id, MAX_MAP_ID_SIZE - 1);
    refine_map_id(new_map_id);

    bool is_map_id_changed = false;
    for (int i = 0; i < MAX_MAP_ID_SIZE - 1; ++i) {
        if (state.map_id[i] != new_map_id[i]) {
            is_map_id_changed = true;
            state.map_id[i] = new_map_id[i];
        }
    }

    if (is_map_id_changed) send_map_id_changed();
}

int patch_memory(void* address, void* patch, size_t size)
{
    DWORD old_protect = 0;

    if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old_protect)) return 0;

    uint8_t* src = (uint8_t*)patch;
    uint8_t* dst = (uint8_t*)address;

    for (size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }

    FlushInstructionCache(GetCurrentProcess(), address, size);
    VirtualProtect(address, size, old_protect, &old_protect);

    return 1;
}

int check_memory(void* address, void* pattern, size_t size)
{
    uint8_t* src = (uint8_t*)pattern;
    uint8_t* dst = (uint8_t*)address;

    for (size_t i = 0; i < size; i++) {
        if (dst[i] != src[i]) {
            return 0;
        }
    }

    return 1;
}

int init_hooks()
{
    uintptr_t base_address = (uintptr_t)GetModuleHandle(0);
    if (!base_address) return 0;

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK) return 0;

    MH_STATUS result;
    #define CREATE_HOOK(name, offset)                                         \
        result = MH_CreateHook(                                               \
            (LPVOID)(base_address + offset),                                  \
            (LPVOID)detour_##name,                                            \
            (LPVOID*)&original_##name                                         \
        );                                                                    \
                                                                              \
        if (result != MH_OK) {                                                \
            return 0;                                                         \
        }                                                                     \
                                                                              \
        result = MH_EnableHook(                                               \
            (LPVOID)(base_address + offset)                                   \
        );                                                                    \
                                                                              \
        if (result != MH_OK) {                                                \
            return 0;                                                         \
        }
    HOOK_LIST(CREATE_HOOK)
    #undef CREATE_HOOK

    return 1;
}

int init_patches()
{
    uintptr_t base_address = (uintptr_t)GetModuleHandle(0);
    if (!base_address) return 0;

#if DS2_32
    // This patch stops the game from giving the dlc keys
    // to the player after character creation in vanilla DS2.
    // It works by making the relevant function return instantly.
    //
    // NOTE (bunnie): I have not verified if the function does anything else.
    // It is possible that this patch might cause issues in the future.
    {
        uintptr_t patch_address = base_address + 0x20FF90;

        uint8_t pattern[] = {0x55}; // PUSH EBP
        if(!check_memory((void*)patch_address, pattern, 1)) return 0;

        uint8_t patch[] = {0xC3};   // RET
        if(!patch_memory((void*)patch_address, patch, 1)) return 0;
    }
#endif

    return 1;
}

#define OVERLAY_PADDING 10.0f
void render_overlay()
{
    static bool movement_locked = true;
    static bool clamp_to_viewport = true;

    static float bg_alpha = 0.4f;
    ImGui::SetNextWindowBgAlpha(bg_alpha);

    ImVec2 default_size(450, 200);
    ImVec2 default_pos(ImGui::GetIO().DisplaySize.x - default_size.x - OVERLAY_PADDING, OVERLAY_PADDING);

    if (ImGui::IsKeyPressed(ImGuiKey_Insert)) {
        ImGui::SetNextWindowPos(default_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(default_size, ImGuiCond_Always);
    } else {
        ImGui::SetNextWindowPos(default_pos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(default_size, ImGuiCond_Once);
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
    if (movement_locked) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }

    ImGui::Begin("Archipelago", nullptr, flags);

    static bool connecting = false;
    char status[1024]; ImVec4 color;
    if (state.ap && state.ap->get_state() == APClient::State::SLOT_CONNECTED) {
        snprintf(status, sizeof(status), "Connected");
        color = ImVec4(0, 1, 0, 1);
        connecting = false;
    }
    else if (state.slot_refused) {
        snprintf(status, sizeof(status), "Slot Refused (%s)", state.slot_refuse_reason);
        color = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        connecting = false;
    }
    else if (connecting) {
        snprintf(status, sizeof(status), "Connecting...");
        color = ImVec4(1, 1, 0, 1);
    }
    else if (state.slot_data_loaded) {
        snprintf(status, sizeof(status), "Reconnecting...");
        color = ImVec4(1, 1, 0, 1);
    }
    else {
        snprintf(status, sizeof(status), "Disconnected");
        color = ImVec4(1, 0, 0, 1);
    }
    ImGui::TextColored(color, "%s", status);

    ImGui::Spacing();

    if (ImGui::BeginTabBar("OverlayTabs")) {

        if (ImGui::BeginTabItem("Connection")) {

            ImGui::BeginDisabled(state.slot_data_loaded);
            ImGui::InputText("Server URI", state.server_uri, MAX_SERVER_URI);
            ImGui::InputText("Slot Name", state.slot_name, MAX_SLOT_NAME);
            ImGui::InputText("Password", state.password, MAX_PASSWORD, ImGuiInputTextFlags_Password);

            ImGui::Spacing();

            if (ImGui::Button("Connect", ImVec2(-1, 0))) {
                // NOTE perhaps race condition
                state.ap = setup_apclient();
                state.slot_refused = 0;
                connecting = true;
            }
            ImGui::EndDisabled();

            ImGui::EndTabItem();
        }

        // TODO log to file
        if (ImGui::BeginTabItem("Console")) {
            ImGui::BeginChild("ConsoleChild", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

            for (size_t i = 0; i < state.console_log.size(); ++i) {
                const auto& line = state.console_log[i];

                ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopTextWrapPos();

                if (i + 1 < state.console_log.size()) {
                    ImGui::Spacing();
                }
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Settings")) {
            ImGui::Checkbox("Lock Overlay Movement (Press Insert to Reset Position)", &movement_locked);

            ImGui::Checkbox("Prevent overlay from going off-screen", &clamp_to_viewport);

            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("##bg_alpha", &bg_alpha, 0.0f, 1.0f, "Background Alpha: %.2f");
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    if (clamp_to_viewport) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        if (viewport->Size.x > 0.0f && viewport->Size.y > 0.0f) {
            ImVec2 pos  = ImGui::GetWindowPos();
            ImVec2 size = ImGui::GetWindowSize();

            float min_x = viewport->Pos.x + OVERLAY_PADDING;
            float min_y = viewport->Pos.y + OVERLAY_PADDING;

            float max_x = viewport->Pos.x + viewport->Size.x - size.x - OVERLAY_PADDING;
            float max_y = viewport->Pos.y + viewport->Size.y - size.y - OVERLAY_PADDING;

            pos.x = (pos.x < min_x) ? min_x : (pos.x > max_x ? max_x : pos.x);
            pos.y = (pos.y < min_y) ? min_y : (pos.y > max_y ? max_y : pos.y);

            ImGui::SetWindowPos(pos);
        }
    }

    ImGui::End();
}


int init()
{
#ifdef MOD_DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#else
    FILE* log_file = freopen("archipelago.log", "a", stdout);
    if (!log_file) {
        DWORD err = GetLastError();
        printf("Error opening log file. Error code: %lu\n", err);
        return 0;
    }
#endif

    if (!CreateDirectoryA("archipelago", 0)) {
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS) {
            printf("Error creating archipelago folder. Error code: %lu\n", err);
            return 0;
        }
    }

    if (!CreateDirectoryA("archipelago\\save_data", 0)) {
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS) {
            printf("Error creating archipelago\\save_data folder. Error code: %lu\n", err);
            return 0;
        }
    }

    if(!libds2_init()) {
        ERROR_PRINT("Error initializing DS2 stuff");
        return 0;
    }
    if(!init_hooks()) {
        ERROR_PRINT("Error initializing hooks");
        return 0;
    }
    if(!init_patches()) {
        ERROR_PRINT("Error initializing patches");
        return 0;
    }
    if(!init_overlay(render_overlay)) {
        ERROR_PRINT("Error initializing overlay");
        return 0;
    }

    // init state
    InitializeCriticalSection(&state.queue_lock);
    state.unused_shop_item.item_id = UNUSED_SHOP_ITEM_ID;
    state.unused_item_count = UNUSED_ITEM_COUNT;
    for (int i = 0; i < UNUSED_ITEM_COUNT; ++i) {
        state.unused_items[i].item_id = UNUSED_ITEM_IDS[i];
    }

#ifdef MOD_DEBUG
    strncpy(state.slot_name, "Player1", sizeof(state.slot_name));
    strncpy(state.server_uri, "localhost:38281", sizeof(state.server_uri));
#endif

    return 1;
}

DWORD WINAPI run(LPVOID)
{
    if (!init()) {
        MessageBoxA(NULL,
                    "An error occurred during initialization.\n"
                    "The mod will disable itself.\n"
                    "Check the archipelago.log file for more information.",
                    "Dark Souls II Archipelago Error",
                    MB_OK | MB_ICONERROR);
    }

    while (!state.stop) {
        if (!state.ap) continue;
        state.ap->poll();
        if (state.ap->get_state() == APClient::State::SLOT_CONNECTED) {
            handle_check_locations();
            handle_give_items();
            handle_current_map_id();

            if (state.goaled) {
                state.ap->StatusUpdate(APClient::ClientStatus::GOAL);
                state.goaled = 0;
            }
        }
        Sleep(1000/60);
    };

    // TODO
    // if (!shutdown()) {
    //     MessageBoxA(NULL,
    //                 "An error occurred during shutdown.\n"
    //                 "Some resources may not have been released properly.",
    //                 "Dark Souls II Archipelago Error",
    //                 MB_OK | MB_ICONERROR);
    // }

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID)
{
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(0, 0, run, 0, 0, 0);
            break;

        case DLL_THREAD_ATTACH: break;
        case DLL_THREAD_DETACH: break;
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
