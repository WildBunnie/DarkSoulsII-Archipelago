from .Items import item_list, ItemData, item_dictionary, item_name_groups
from .Locations import locations_by_region, LocationData, location_name_groups
from .Options import DS2Options
from .Regions import region_map, region_list, RegionData
from .Enums import DLC, ItemCategory
from .Rules import connection_rules

from worlds.AutoWorld import World, WebWorld
from worlds.generic.Rules import set_rule, add_item_rule, add_rule
from BaseClasses import Item, ItemClassification, Location, Region, LocationProgressType, Tutorial, MultiWorld

from typing import List, Optional, Set

class DS2Location(Location):
    game: str = "Dark Souls II"
    data: Optional[LocationData]

    def __init__(self, player, name, address, parent, data):
        self.data = data
        super(DS2Location, self).__init__(
            player, name, address, parent
        )

class DS2Item(Item):
    game: str = "Dark Souls II"
    data: Optional[ItemData]

    def __init__(self, name, classification, code, player, data):
        self.data = data
        super(DS2Item, self).__init__(
            name, classification, code, player
        )

class DarkSouls2Web(WebWorld):
    theme = "stone"
    setup_en = Tutorial(
        "Multiworld Setup Guide",
        "A guide to setting up the Archipelago Dark Souls II randomizer on your computer.",
        "English",
        "setup_en.md",
        "setup/en",
        ["WildBunnie"]
    )

    tutorials = [setup_en]

class DS2World(World):
    game = "Dark Souls II"

    options_dataclass = DS2Options
    options: DS2Options

    item_name_to_id = {item_data.name: item_data.code 
                       for item_data in item_list}

    location_name_to_id = {location_data.name: location_data.address 
                           for locations in locations_by_region.values() 
                           for location_data in locations 
                           if location_data.address != None}

    item_name_groups = item_name_groups
    location_name_groups = location_name_groups

    def generate_early(self):
        if self.options.early_blacksmith == "early_global":
            self.multiworld.early_items[self.player]["Lenigrast's Key"] = 1
        elif self.options.early_blacksmith == "early_local":
            self.multiworld.local_early_items[self.player]["Lenigrast's Key"] = 1

    def create_region(self, name: str) -> Region:
        return Region(name, self.player, self.multiworld)

    def create_location(self, name: str, region: Region, data: LocationData) -> DS2Location:
        location = DS2Location(self.player, name, self.location_name_to_id[name], region, data)
        if data.missable: location.progress_type = LocationProgressType.EXCLUDED
        return location

    def is_region_enabled(self, region_data: RegionData):

        if region_data.dlc == DLC.SUNKEN_KING and not self.options.sunken_king_dlc: return False
        if region_data.dlc == DLC.OLD_IRON_KING and not self.options.old_iron_king_dlc: return False
        if region_data.dlc == DLC.IVORY_KING and not self.options.ivory_king_dlc: return False

        return True

    def create_regions(self):
        regions: dict[str, Region] = {}

        menu_region = self.create_region("Menu")
        self.multiworld.regions.append(menu_region)
        regions["Menu"] = menu_region

        # create regions
        for region_name in locations_by_region:
            
            assert region_name in region_map, f"Region data not found for region {region_name}"
            region_data = region_map[region_name]

            if not self.is_region_enabled(region_data): continue

            region = self.create_region(region_name)
            regions[region_name] = region
            self.multiworld.regions.append(region)
            
            # create locations for region
            for location_data in locations_by_region[region_name]:
                if location_data.sotfs and not self.options.game_version == "sotfs": continue
                if location_data.vanilla and not self.options.game_version == "vanilla": continue

                if location_data.event:
                    location = DS2Location(self.player, location_data.name, None, region, None)
                    region.locations.append(location)
                    continue

                if location_data.name in self.options.remove_locations.value:
                    item_classification = item_dictionary[location_data.original_item_name].classification
                    is_progression = item_classification in [
                        ItemClassification.progression,
                        ItemClassification.progression_skip_balancing,
                    ]
                    
                    if not is_progression: continue

                location = self.create_location(location_data.name, region, location_data)
                region.locations.append(location)
        
        # connect regions
        for region_name, region in regions.items():
            region_data = region_map[region_name]
            if not region_data.connections: continue
            for connection in region_data.connections:
                if not connection in regions.keys(): continue
                region.connect(regions[connection])
    
    def create_item(self, name: str) -> DS2Item:
        assert name in item_dictionary, f"Tried to create an item that doesn't exist {name}"
        item_data: ItemData = item_dictionary[name]

        if item_data.max_reinforcement > 0 and self.random.randint(0, 99) < self.options.randomize_weapon_level_percentage:
            if item_data.max_reinforcement == 5:
                item_data.reinforcement = self.random.randint(self.options.min_weapon_reinforcement_in_5, self.options.max_weapon_reinforcement_in_5)
            if item_data.max_reinforcement == 10:
                item_data.reinforcement = self.random.randint(self.options.min_weapon_reinforcement_in_10, self.options.max_weapon_reinforcement_in_10)

        return DS2Item(name, item_data.classification, self.item_name_to_id[name], self.player, item_data)

    def create_items(self):
        # place events in their locations
        events = [location_data for locations in locations_by_region.values() for location_data in locations if location_data.event]
        for event in events:
            event_item = DS2Item(event.name, ItemClassification.progression, None, self.player, None)
            self.multiworld.get_location(event.name, self.player).place_locked_item(event_item)

        pool : list[DS2Item] = []
        max_pool_size = len(self.multiworld.get_unfilled_locations(self.player))

        # fill pool with all the original items from each location
        items_in_pool = [item.name for item in pool]
        locations: List[DS2Location] = self.multiworld.get_unfilled_locations(self.player)
        for location in locations:
            if location.data.keep_original_item:
                location.place_locked_item(self.create_item(location.data.original_item_name))
                max_pool_size -= 1
                continue
            
            item_name = location.data.original_item_name
            assert item_name in item_dictionary, f"item '{item_name}' from location '{location.data.name}' doesn't exist"

            item_data = item_dictionary[item_name]
            if item_data.skip: continue
            if item_data.category == ItemCategory.UNIQUE and item_data.name in items_in_pool: continue

            item = self.create_item(item_data.name)
            items_in_pool.append(item_data.name)
            pool.append(item)

        assert len(pool) <= max_pool_size, f"item pool is over-filled by {abs(len(pool) - max_pool_size)}"

        # fill the rest of the pool
        filler_items = [item.name for item in item_list if item.category == ItemCategory.GOOD and not item.skip and not item.sotfs and not item.dlc]
        for _ in range(max_pool_size - len(pool)):
            item_name = self.random.choice(filler_items)
            item = self.create_item(item_name)
            pool.append(item)

        assert len(pool) == max_pool_size, f"item pool is under-filled by {abs(len(pool) - max_pool_size)}"
        self.multiworld.itempool += pool

    def set_item_rules(self):
        for location in self.multiworld.get_locations(self.player):
            if location.data and location.data.shop:
                add_item_rule(location, lambda item: 
                              (item.player != self.player or
                              item.data.bundle == False))
    
    def set_connection_rules(self):
        for rule in connection_rules:
            if "game_version" in rule and rule["game_version"] != self.options.game_version:
                continue

            from_region = rule["from"]
            to_region = rule["to"]
            state_func = rule["state"]

            enabled = True
            for region in [from_region, to_region]:
                assert region in region_map, f"[set_connection_rules] region '{region}' in connection rule does not exist"
                region_data = region_map[region]
                if not self.is_region_enabled(region_data): 
                    enabled = False

            if not enabled: continue

            state_check = lambda state, func=state_func: func(state, self.player)
            add_rule(
                self.multiworld.get_entrance(f"{from_region} -> {to_region}", self.player),
                state_check
            )

    def set_rules(self):
        
        self.set_item_rules()
        # self.set_connection_rules()

        # set_rule(self.multiworld.get_location("TW: Soul of Nashandra", self.player), lambda state: state.has("Giant's Kinship", self.player))
        # self.multiworld.completion_condition[self.player] = lambda state: state.has("TW: Soul of Nashandra", self.player)

    def fill_slot_data(self) -> dict:
        slot_data = self.options.as_dict("death_link","game_version","no_weapon_req","no_spell_req","no_armor_req","no_equip_load","randomize_starting_loadout", "starting_weapon_requirement", "autoequip")

        # non archipelago locations that should not be randomized by the static randomizer
        keep_unrandomized = set()

        # [Event - Giant Memories] Soul of a Giant
        keep_unrandomized.add(260004000) 
        
        # [Merchant Hag Melentia - Majula] Lifegem
        if self.options.infinite_lifegems:
            keep_unrandomized.add(375400601)

        # straid trades
        keep_unrandomized.update([
            376801000, 376801001, 376801002, 376801003, 376801004, 376801005,
            376801006, 376801007, 376801008, 376801009, 376801010, 376801011,
            376801012, 376801013, 376801014, 376801015, 376801016, 376801017,
            376801100, 376801101, 376801102, 376801103, 376801104, 376801105,
            376801106, 376801107, 376801108, 376801109, 376801110, 376801111,
            376801200, 376801201, 376801202, 376801203, 376801204, 376801205,
            376801206, 376801207, 376801208, 376801209, 376801210, 376801211,
            376801212, 376801213, 376801214, 376801215, 376801216, 376801217,
            376801300, 376801301, 376801302, 376801303, 376801304, 376801305,
            376801306    
        ])

        # ornifex trades
        keep_unrandomized.update([
            377601000, 377601001, 377601002, 377601003, 377601004, 377601005, 377601006, 377601007,
            377601008, 377601009, 377601010, 377601011, 377601012, 377601013, 377601014, 377601015,
            377601016, 377601017, 377601018, 377601019, 377601020, 377601021, 377601022, 377601023,
            377601024, 377601025, 377601026, 377601027, 377601028, 377601029, 377601030, 377601031,
            377601032, 377601033, 377601034, 377601035, 377601036, 377601037, 377601100, 377601101,
            377601102, 377601103, 377601104, 377601105, 377601106, 377601107, 377601108, 377601109,
            377601110, 377601111, 377601112, 377601113, 377601114, 377601115, 377601116, 377601117,
            377601118, 377601119, 377601120, 377601121, 377602000, 377602001, 377602002, 377602003,
            377602004, 377602005, 377602006, 377602007, 377602008, 377602009, 377602010, 377602011,
            377602012, 377602013, 377602014, 377602015, 377602016, 377602017, 377602018, 377602019,
            377602020, 377602021, 377602022, 377602023, 377602024, 377602025, 377602026, 377602027,
            377602028, 377602029, 377602030, 377602031, 377602032, 377602033, 377602034, 377602035,
            377602036, 377602037, 377602100, 377602101, 377602102, 377602103, 377602104, 377602105,
            377602106, 377602107, 377602108, 377602109, 377602110, 377602111, 377602112, 377602113,
            377602114, 377602115, 377602116, 377602117, 377602118, 377602119, 377602120, 377602121
        ])

        # bird trades
        keep_unrandomized.update([
            250000000, 250000001, 250000002, 250000100,
            250000101, 250000102, 250000202, 250000303
        ])

        # new game plus
        keep_unrandomized.update([
            101040500, 101190001, 101193000, 102210501, 102210601, 102210602, 102213001, 184700000, 
            184710000, 200154001, 200309701, 200324001, 200326001, 200332001, 200333001, 200501001, 
            200503001, 200504001, 200603001, 200607001, 200619101, 200626001, 200675010, 200862001, 
            210026001, 210026031, 210045001, 210045002, 210105021, 210105041, 210106061, 210106321, 
            210106371, 210145061, 210146051, 210146181, 210146381, 210156031, 210156161, 210165041, 
            210166191, 210166421, 210166441, 210175021, 210176171, 210176221, 210176231, 210176461, 
            210185001, 210185071, 210185081, 210186021, 210186071, 210195001, 210196111, 210196211, 
            210236021, 210236071, 210236131, 210275021, 210276041, 210276061, 210315001, 210316041, 
            210316101, 210325001, 210326081, 210326101, 210326141, 210326191, 210335021, 210335031, 
            210336011, 210336041, 210346031, 210346091, 220105001, 220106011, 220106061, 220106111, 
            220106141, 220115051, 220116011, 220116171, 220215011, 220215021, 220215041, 220216021, 
            220216061, 220246011, 220246111, 220246121, 220246151, 260008110, 260044001, 372110007, 
            372110008, 372110105, 372110300, 372110301, 372110302, 376100259, 376100260, 376100261, 
            376100262, 377200208, 377200209, 377200210, 377200211, 378300603, 378500603
        ])

        # maughlin restocks
        keep_unrandomized.update([376100219, 376100220, 376100221, 376100222, 376100223, 376100224, 376100225, 376100226])

        ap_to_location_id = {}
        location_to_ap_id = {}
        for locations in locations_by_region.values():
            for location_data in locations:
                if not isinstance(location_data.address, int): continue # skip events
                
                # archipelago to location id
                ap_to_location_id[location_data.address] = location_data.location_id
                
                # location to archipelago id
                if location_data.location_id not in location_to_ap_id:
                    location_to_ap_id[location_data.location_id] = [location_data.address]
                else:
                    location_to_ap_id[location_data.location_id].append(location_data.address)
        
        item_bundles = set()
        reinforcements = {}
        for item in item_list:
            if item.bundle: 
                item_bundles.add(item.code)
            if item.reinforcement != 0:
                reinforcements[item.code] = item.reinforcement

        slot_data["keep_unrandomized"] = keep_unrandomized
        slot_data["ap_to_location_id"] = ap_to_location_id
        slot_data["location_to_ap_id"] = location_to_ap_id
        slot_data["item_bundles"] = item_bundles
        slot_data["reinforcements"] = reinforcements

        return slot_data
