from .Items import item_list, ItemData, item_dictionary, item_name_groups
from .Locations import locations_by_region, LocationData, location_name_groups
from .Options import DS2Options
from .Regions import region_map, region_list
from .Enums import DLC, ItemCategory

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

    def create_regions(self):
        regions: dict[str, Region] = {}

        menu_region = self.create_region("Menu")
        self.multiworld.regions.append(menu_region)
        regions["Menu"] = menu_region

        # create regions and locations
        for region_name in locations_by_region:
            
            assert region_name in region_map, f"Region data not found for region {region_name}"
            region_data = region_map[region_name]

            if region_data.dlc == DLC.SUNKEN_KING and not self.options.sunken_king_dlc: continue
            if region_data.dlc == DLC.OLD_IRON_KING and not self.options.old_iron_king_dlc: continue
            if region_data.dlc == DLC.IVORY_KING and not self.options.ivory_king_dlc: continue

            region = self.create_region(region_name)
            regions[region_name] = region
            self.multiworld.regions.append(region)
            
            for location_data in locations_by_region[region_name]:
                if location_data.sotfs and not self.options.game_version == "sotfs": continue
                if location_data.vanilla and not self.options.game_version == "vanilla": continue

                if location_data.event:
                    location = DS2Location(self.player, location_data.name, None, region, None)
                    region.locations.append(location)
                    continue

                is_excluded = location_data.name in self.options.exclude_locations.value
                excluded_behavior = self.options.excluded_location_behavior

                if is_excluded and excluded_behavior == "do_not_create":
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

    def set_rules(self):
        
        # dont allow certain stuff in shops
        for location in self.multiworld.get_locations(self.player):
            if location.data and location.data.shop:
                add_item_rule(location, lambda item: 
                              (item.player != self.player or
                              item.data.bundle == False))

        # allow anything but progression items in excluded locations if using "allow_useful"
        if self.options.excluded_location_behavior == "allow_useful":
            locations: List[DS2Location] = self.multiworld.get_locations(self.player)
            for location in locations:
                if location.data.keep_original_item == True: continue
                if location.name in self.options.exclude_locations.value:
                    add_item_rule(location, lambda item: not item.advancement)
            
            self.options.exclude_locations.value.clear()


    # set_rule(self.multiworld.get_location("Defeat Nashandra", self.player), lambda state: state.has("Giant's Kinship", self.player))
    # self.multiworld.completion_condition[self.player] = lambda state: state.has("Defeat Nashandra", self.player)

    #     self.set_shop_rules()

    #     # EVENTS
    #     self.set_location_rule("Rotate the Majula Rotunda", lambda state: state.has("Rotunda Lockstone", self.player))
    #     self.set_location_rule("Open Shrine of Winter", lambda state: 
    #         (state.has("Defeat the Rotten", self.player) and
    #          state.has("Defeat the Lost Sinner", self.player) and
    #          state.has("Defeat the Old Iron King", self.player) and
    #          state.has("Defeat the Duke's Dear Freja", self.player)))

    #     # LOCATIONS
    #     ## ALDIA KEY
    #     self.set_location_rule("[AldiasKeep] Inside a barrel in the corner in side room with caged Gargoyle", lambda state: state.has("Aldia Key", self.player))
    #     self.set_location_rule("[AldiasKeep] On table in side room with caged Gargoyle", lambda state: state.has("Aldia Key", self.player))
    #     ## MAJULA
    #     self.set_location_rule("[Majula] Wooden chest in Lenigrast's workshop", lambda state: state.has("Lenigrast's Key", self.player))
    #     self.set_location_rule("[Majula] Library room in Cale's house", lambda state: state.has("House Key", self.player))
    #     self.set_location_rule("[Majula] Corpse in Cale's house basement", lambda state: state.has("House Key", self.player))
    #     self.set_location_rule("[Majula] Metal chest in Cale's house basement", lambda state: state.has("House Key", self.player))
    #     self.set_location_rule("[Majula] Wooden chest on the attic of Majula mansion", lambda state: state.has("House Key", self.player))
    #     ## LOWER FIRE AREA
    #     self.set_location_rule("[FOFG] First corpse in the lower fire area", lambda state: state.has("Iron Key", self.player))
    #     self.set_location_rule("[FOFG] Second corpse in the lower fire area", lambda state: state.has("Iron Key", self.player))
    #     ## FANG KEY
    #     self.set_location_rule("[ShadedWoods] Room where Ornifex is locked", lambda state: state.has("Fang Key", self.player))
    #     ## TSELDORA DEN
    #     self.set_location_rule("[Tseldora] Metal chest in Tseldora den", lambda state: state.has("Tseldora Den Key", self.player))
    #     self.set_location_rule("[Tseldora] Wooden chest in Tseldora den", lambda state: state.has("Tseldora Den Key", self.player))
    #     self.set_location_rule("[Tseldora] Metal chest behind locked door in pickaxe room", lambda state: state.has("Brightstone Key", self.player))
    #     ## FORGOTTEN KEY
    #     self.set_location_rule("[Pit] First metal chest behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
    #     self.set_location_rule("[Pit] Third metal chest behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
    #     self.set_location_rule("[Pit] Second metal chest behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
    #     if self.options.game_version == "sotfs": 
    #         self.set_location_rule("[Pit] Corpse behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
    #     self.set_location_rule("[Gutter] Urn behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
    #     ## BASTILLE KEY
    #     self.set_location_rule("[Bastille] In a cell next to Straid's cell", lambda state: state.has("Bastille Key", self.player))
    #     self.set_location_rule("[SinnersRise] In locked cell left side upper level", lambda state: state.has("Bastille Key", self.player))
    #     self.set_location_rule("[SinnersRise] In right side oil-sconce room just before the Sinner", lambda state: state.has("Bastille Key", self.player))
    #     if self.options.enable_ngp:
    #         self.set_location_rule("[Bastille] In a cell next to Straid's cell in NG+", lambda state: state.has("Bastille Key", self.player))
    
    #     self.set_location_rule("[ShadedWoods] Gift from Manscorpion Tark after defeating Najka", lambda state: state.has("Ring of Whispers", self.player))
    #     ## VENDRICK
    #     self.set_location_rule("[Amana] On a throne behind a door that opens after defeating vendrick", lambda state: state.has("Soul of a Giant", self.player, 5))
    #     self.set_location_rule("[Amana] Metal chest behind a door that opens after defeating vendrick", lambda state: state.has("Soul of a Giant", self.player, 5))
    #     ## SKELETON LORDS
    #     self.set_location_rule("Defeat the Skeleton Lords", lambda state: state.has("Undead Lockaway Key", self.player))
    #     self.set_location_rule("[Copse] Skeleton Lords drop", lambda state: state.has("Defeat the Skeleton Lords", self.player))
    #     if self.options.enable_ngp:
    #         self.set_location_rule("[Copse] Skeleton Lords drop in NG+", lambda state: state.has("Defeat the Skeleton Lords", self.player))

    #     #STATUES
    #     if self.options.game_version == "sotfs":
    #         self.set_location_rule("[Betwixt] In the basilisk pit", lambda state: state.has("Unpetrify Statue in Things Betwixt", self.player))
    #         self.set_location_rule("[Heides] Metal chest behind petrified hollow after Dragonrider", lambda state: state.has("Unpetrify Statue in Heide's Tower of Flame", self.player))
    #         self.set_location_rule("[Heides] On railing behind petrified hollow", lambda state: state.has("Unpetrify Statue in Heide's Tower of Flame", self.player))
    #         self.set_location_rule("Defeat the Rotten", lambda state: state.has("Unpetrify Statue in Black Gulch", self.player))
    #         self.set_location_rule("[Gulch] Urn next to the second bonfire", lambda state: state.has("Unpetrify Statue in Black Gulch", self.player))
    #         self.set_location_rule("[ShadedWoods] Metal chest blocked by petrified statue", lambda state: state.has("Unpetrify Statue Blocking the Chest in Shaded Ruins", self.player))
    #         self.set_location_rule("[ShadedWoods] Drop from Petrified Lion Warrior next to Golden Lion Warrior", lambda state: state.has("Unpetrify Warlock Mask Statue in Shaded Ruins", self.player))
    #         self.set_location_rule("[ShadedWoods] Corpse next to chest in area behind two petrified statues and Vengarl's body", lambda state: state.has("Unpetrify Statue near Manscorpion Tark" or
    #                                                                                                "Unpetrify Statue near Black Knight Halberd", self.player))
    #         self.set_location_rule("[ShadedWoods] Next to Vengarl's body", lambda state: state.has("Unpetrify Statue near Manscorpion Tark" or
    #                                                                                                "Unpetrify Statue near Black Knight Halberd", self.player))
    #         self.set_location_rule("[AldiasKeep] Drop from Petrified Undead Traveller just before Giant Basilisk", lambda state: state.has("Unpetrify Left Cage Statue in Aldia's Keep", self.player))
    #         self.set_location_rule("[AldiasKeep] Drop from Centre petrified Undead Traveller just before Giant Basilisk", lambda state: state.has("Unpetrify Right Cage Statue in Aldia's Keep", self.player))
    #     self.set_location_rule("[ShadedWoods] Metal chest in room blocked by petrified statue", lambda state: state.has("Unpetrify Lion Mage Set Statue in Shaded Ruins", self.player))
    #     self.set_location_rule("[ShadedWoods] Drop from the petrified lion warrior by the tree bridge", lambda state: state.has("Unpetrify Fang Key Statue in Shaded Ruins", self.player))
    #     self.set_location_rule("[AldiasKeep] Drop from Petrified Ogre blocking stairway near Bone Dragon", lambda state: state.has("Unpetrify Cyclops Statue in Aldia's Keep", self.player))

    #     # lockstones
    #     self.set_location_rule("[FOFG] First metal chest behind Pharros' contraption under the ballista-trap", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[FOFG] Second metal chest behind Pharros' contraption under the ballista-trap", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[Bastille] Wooden chest behind Pharros' contraption in Pharros/elevator room", lambda state: state.has("Master Lockstone", self.player))
    #     if self.options.game_version == "vanilla":
    #         self.set_location_rule("[Bastille] Metal chest next to elevator in Pharros/elevator room", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[EarthernPeak] Metal chest behind Pharros contraption in the lowest level next to Lucatiel", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[DragonShrine] Metal chest behind the Pharros contraption under the staircase", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[Pharros] Wooden chest in room after using top Pharros contraption and dropping down near the toxic rats", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[Pharros] Trapped wooden chest behind (floor) Pharros contraption in the upper level", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[Pharros] Corpse- behind three-part Pharros' door in the upper level", lambda state: state.has("Master Lockstone", self.player))
    #     if self.options.enable_ngp:
    #         self.set_location_rule("[Pharros] Trapped wooden chest behind (floor) Pharros contraption in the upper level in NG+", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[Pharros] Metal chest behind three-part pharros door in the lower level", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[MemoryOrro] Trapped wooden chest behind a Pharros' contraption on the second floor", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[MemoryOrro] Metal chest behind a Pharros contraption and an illusory wall on the second floor", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[MemoryOrro] Metal chest behind a Pharros contraption and an illusory wall on the second floor (2)", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[Amana] Metal chest behind a pharros contraption near the crumbled ruins bonfire", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[Crypt] Metal chest behind a illusory wall and a Pharros contraption from the third graveyard room", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[GraveOfSaints] 1st floor on other side of the drawbridges", lambda state: state.has("Master Lockstone", self.player))
    #     self.set_location_rule("[GraveOfSaints] 2nd floor on other side of the drawbridges", lambda state: state.has("Master Lockstone", self.player))

    #     # kings ring 
    #     self.set_location_rule("[FOFG] On scaffolding near the Place Unbeknownst bonfire", lambda state: state.has("King's Ring", self.player))

    #     # CONNECTIONS
        
    #     if self.options.sunken_king_dlc:
    #         if self.options.game_version == "sotfs": self.set_connection_rule("The Gutter", "Shulva", lambda state: state.has("Dragon Talon", self.player))
    #         self.set_connection_rule("Shulva", "Shulva - Sanctum Key", lambda state: state.has("Eternal Sanctum Key", self.player))
    #         self.set_connection_rule("Shulva", "Shulva - Dragon Stone", lambda state: state.has("Dragon Stone", self.player))
    #     if self.options.old_iron_king_dlc:
    #         if self.options.game_version == "sotfs": self.set_connection_rule("Iron Keep", "Brume Tower", lambda state: state.has("Heavy Iron Key", self.player))
    #         self.set_connection_rule("Brume Tower", "Brume Tower - scepter", lambda state: state.has("Scorching Iron Scepter", self.player))
    #         self.set_connection_rule("Brume Tower - scepter", "Iron Passage", lambda state: state.has("Tower Key", self.player))
    #         self.set_connection_rule("Iron Passage", "Memory of the Old Iron King", lambda state: state.has("Ashen Mist Heart", self.player))
    #         self.set_location_rule("[DLC2] Wooden chest in the left of the dark cursed area next to the Foyer bonfire", lambda state: state.has("Tower Key", self.player))
    #         self.set_location_rule("[DLC2] Metal chest in the dark cursed area next to the Foyer bonfire", lambda state: state.has("Tower Key", self.player))
    #         self.set_location_rule("[DLC2] On altar in the dark cursed area next to the Foyer bonfire", lambda state: state.has("Tower Key", self.player))
    #         self.set_location_rule("[DLC2] Fume Knight drop", lambda state: state.has("Smelter Wedge x11", self.player))
    #         if self.options.enable_ngp: self.set_location_rule("[DLC2] Fume Knight drop in NG+", lambda state: state.has("Smelter Wedge x11", self.player))
    #     if self.options.ivory_king_dlc:
    #         if self.options.game_version == "sotfs": self.set_connection_rule("Drangleic Castle", "Eleum Loyce", lambda state: state.has("Frozen Flower", self.player))
    #         self.set_connection_rule("Eleum Loyce", "Frigid Outskirts", lambda state: state.has("Garrison Ward Key", self.player))

    #     self.set_connection_rule("Huntsman's Copse", "Earthen Peak", lambda state: state.has("Defeat the Skeleton Lords", self.player))
    #     self.set_connection_rule("Forest of Fallen Giants", "FOFG - Soldier Key", lambda state: state.has("Soldier Key", self.player))
    #     self.set_connection_rule("Forest of Fallen Giants", "FOFG - Salamander Pit", lambda state: state.has("Iron Key", self.player))
    #     self.set_connection_rule("Shaded Woods", "Aldia's Keep", lambda state: state.has("King's Ring", self.player))
    #     self.set_connection_rule("Shaded Woods", "Drangleic Castle", lambda state: state.has("Open Shrine of Winter", self.player))
    #     self.set_connection_rule("Drangleic Castle", "Dark Chasm of Old", lambda state: state.has("Forgotten Key", self.player))
    #     self.set_connection_rule("Drangleic Castle", "King's Passage", lambda state: state.has("Key to King's Passage", self.player))
    #     self.set_connection_rule("Forest of Fallen Giants", "Memory of Vammar", lambda state: state.has("Ashen Mist Heart", self.player))
    #     self.set_connection_rule("FOFG - Soldier Key", "Memory of Orro", lambda state: state.has("Ashen Mist Heart", self.player))
    #     self.set_connection_rule("FOFG - Soldier Key", "Memory of Jeigh", lambda state: 
    #                                 state.has("King's Ring", self.player) and 
    #                                 state.has("Ashen Mist Heart", self.player))
    #     self.set_connection_rule("Drangleic Castle", "Throne of Want", lambda state: state.has("King's Ring", self.player))
    #     self.set_connection_rule("Iron Keep", "Belfry Sol", lambda state: state.has("Master Lockstone", self.player))

    #     # LOST BASTILLE
    #     self.set_connection_rule("Lost Bastille - Wharf", "Lost Bastille - After Key", lambda state: state.has("Antiquated Key", self.player))
    #     self.set_connection_rule("Lost Bastille - After Statue", "Belfry Luna", lambda state: state.has("Master Lockstone", self.player))
    #     if self.options.game_version == "sotfs": 
    #         self.set_connection_rule("Early Lost Bastille", "Lost Bastille - After Statue", lambda state: state.has("Unpetrify Statue in Lost Bastille", self.player))
    #     elif self.options.game_version == "vanilla":
    #         self.set_connection_rule("Lost Bastille - After Key", "Late Lost Bastille", lambda state: state.has("Master Lockstone", self.player))
        
    #     #This is down here because we don't want to have the rules get overwritten from putting it before, since it adds rules on top of pre-existing connections
    #     self.add_combat_rules()

    #     # from Utils import visualize_regions
    #     # visualize_regions(self.multiworld.get_region("Menu", self.player), "my_world.puml")

    # #set/overwrites rules
    # def set_connection_rule(self, fromRegion, toRegion, state):
    #     set_rule(self.multiworld.get_entrance(f"{fromRegion} -> {toRegion}", self.player), state)

    # def set_location_rule(self, name, state):
    #     set_rule(self.multiworld.get_location(name, self.player), state)
    
    # #adds new rules on top of pre-existing rules
    # def add_connection_rule(self, fromRegion, toRegion, state):
    #     add_rule(self.multiworld.get_entrance(f"{fromRegion} -> {toRegion}", self.player), state)
    
    # def add_location_rule(self, name, state):
    #     add_rule(self.multiworld.get_location(name, self.player), state)

    # def set_shop_rules(self):
    #     self.set_location_rule("[Sweet Shalquoir - Royal Rat Authority, Royal Rat Vanguard] Flying Feline Boots", lambda state: 
    #                            state.has("Defeat the Royal Rat Authority", self.player) and state.has("Defeat the Royal Rat Vanguard", self.player))
    #     self.set_location_rule("[Lonesome Gavlan - Harvest Valley] Ring of Giants", lambda state: state.has("Speak with Lonesome Gavlan in No-man's Wharf", self.player))       
    #     if self.options.game_version == "sotfs":
    #         self.set_location_rule("[Head of Vengarl] Red Rust Scimitar", lambda state: state.has("Unpetrify Statue near Manscorpion Tark", self.player))
    #         self.set_location_rule("[Head of Vengarl] Red Rust Shield", lambda state: state.has("Unpetrify Statue near Manscorpion Tark", self.player))
    #         self.set_location_rule("[Head of Vengarl] Red Rust Sword", lambda state: state.has("Unpetrify Statue near Manscorpion Tark", self.player))

    #     for region in locations_by_region:
    #         for location in locations_by_region[region]:
    #             if "[Laddersmith Gilligan - Majula]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Defeat Mytha, the Baneful Queen", self.player))
    #             elif "[Rosabeth of Melfia]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Unpetrify Rosabeth of Melfia", self.player))
    #             elif "[Blacksmith Lenigrast]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Lenigrast's Key", self.player))
    #             elif "[Steady Hand McDuff]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Dull Ember", self.player))
    #             elif "[Lonesome Gavlan - Doors of Pharros]" in locations_by_region:
    #                 self.set_location_rule(location.name, lambda state: state.has("Speak with Lonesome Gavlan in Harvest Valley", self.player))
    #             elif "Straid of Olaphis" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Unpetrify Straid of Olaphis", self.player))
    #             elif " - Shrine of Winter]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Open Shrine of Winter", self.player))
    #             elif " - Skeleton Lords]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Defeat the Skeleton Lords", self.player))
    #             elif " - Looking Glass Knight]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Defeat the Looking Glass Knight", self.player))
    #             elif " - Lost Sinner]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Defeat the Lost Sinner", self.player))
    #             elif " - Old Iron King]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Defeat the Old Iron King", self.player))
    #             elif " - Velstadt]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Defeat Velstadt", self.player))
    #             elif " - Smelter Demon]" in location.name:
    #                 self.set_location_rule(location.name, lambda state: state.has("Defeat the Smelter Demon", self.player))

    # def add_combat_rules(self):

    #     if self.options.combat_logic == "easy":
        
    #         #Lost Sinner Route
    #         self.add_connection_rule("Forest of Fallen Giants", "FOFG - Salamander Pit", lambda state: state.has("Estus Flask Shard", self.player, 6) and state.has("Sublime Bone Dust", self.player, 3))
    #         self.add_connection_rule("FOFG - Soldier Key", "Lost Bastille - FOFG", lambda state: state.has("Estus Flask Shard", self.player, 3) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Heide's Tower of Flame", "Cathedral of Blue", lambda state: state.has("Estus Flask Shard", self.player, 3) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("No-man's Wharf", "Lost Bastille - Wharf", lambda state: state.has("Estus Flask Shard", self.player, 3) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Late Lost Bastille", "Sinners' Rise", lambda state: state.has("Estus Flask Shard", self.player, 7) and state.has("Sublime Bone Dust", self.player, 3))
    #         #Old Iron King Route
    #         self.add_location_rule("Defeat the Skeleton Lords", lambda state: state.has("Estus Flask Shard", self.player, 4) and state.has("Sublime Bone Dust", self.player, 2))
    #         self.add_location_rule("[Chariot] Executioner's Chariot drop", lambda state: state.has("Estus Flask Shard", self.player, 2) and state.has("Sublime Bone Dust", self.player, 1))
    #         if self.options.enable_ngp:
    #             self.add_location_rule("[Chariot] Executioner's Chariot drop in NG+", lambda state: state.has("Estus Flask Shard", self.player, 2) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_location_rule("[Chariot] Above the stairs leading to the bonfire", lambda state: state.has("Estus Flask Shard", self.player, 2) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Earthen Peak", "Iron Keep", lambda state: state.has("Estus Flask Shard", self.player, 7) and state.has("Sublime Bone Dust", self.player, 3))
    #         #The Rotten Route
    #         self.add_connection_rule("Majula", "Grave of Saints", lambda state: state.has("Estus Flask Shard", self.player, 3) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Grave of Saints", "The Gutter", lambda state: state.has("Estus Flask Shard", self.player, 5) and state.has("Sublime Bone Dust", self.player, 2))
    #         #Duke's Dear Freja Route(especially the Royal Rat Authority)
    #         self.add_connection_rule("Shaded Woods", "Doors of Pharros", lambda state: state.has("Estus Flask Shard", self.player, 5) and state.has("Sublime Bone Dust", self.player, 2))
    #         self.add_connection_rule("Doors of Pharros", "Brightstone Cove", lambda state: state.has("Estus Flask Shard", self.player, 7) and state.has("Sublime Bone Dust", self.player, 3))
    #         #Late game
    #         self.add_connection_rule("Shaded Woods", "Drangleic Castle", lambda state: state.has("Estus Flask Shard", self.player, 9) and state.has("Sublime Bone Dust", self.player, 4))
    #         self.add_connection_rule("Shaded Woods", "Aldia's Keep", lambda state: state.has("Estus Flask Shard", self.player, 9) and state.has("Sublime Bone Dust", self.player, 4))
    #         if self.options.sunken_king_dlc:
    #             self.add_connection_rule("The Gutter", "Shulva", lambda state: state.has("Estus Flask Shard", self.player, 12) and state.has("Sublime Bone Dust", self.player, 5))
    #         if self.options.old_iron_king_dlc:
    #             self.add_connection_rule("Iron Keep", "Brume Tower", lambda state: state.has("Estus Flask Shard", self.player, 12) and state.has("Sublime Bone Dust", self.player, 5))
    #         if self.options.ivory_king_dlc:
    #             self.add_connection_rule("Drangleic Castle", "Eleum Loyce", lambda state: state.has("Estus Flask Shard", self.player, 12) and state.has("Sublime Bone Dust", self.player, 5))
        
    #     if self.options.combat_logic == "medium":
        
    #         #Lost Sinner Route
    #         self.add_connection_rule("Forest of Fallen Giants", "FOFG - Salamander Pit", lambda state: state.has("Estus Flask Shard", self.player, 4) and state.has("Sublime Bone Dust", self.player, 2))
    #         self.add_connection_rule("FOFG - Soldier Key", "Lost Bastille - FOFG", lambda state: state.has("Estus Flask Shard", self.player, 1) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Heide's Tower of Flame", "Cathedral of Blue", lambda state: state.has("Estus Flask Shard", self.player, 2) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("No-man's Wharf", "Lost Bastille - Wharf", lambda state: state.has("Estus Flask Shard", self.player, 1) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Late Lost Bastille", "Sinners' Rise", lambda state: state.has("Estus Flask Shard", self.player, 5) and state.has("Sublime Bone Dust", self.player, 2))
    #         #Old Iron King Route
    #         self.add_location_rule("Defeat the Skeleton Lords", lambda state: state.has("Estus Flask Shard", self.player, 3) and state.has("Sublime Bone Dust", self.player, 2))
    #         self.add_location_rule("[Chariot] Executioner's Chariot drop", lambda state: state.has("Estus Flask Shard", self.player, 2) and state.has("Sublime Bone Dust", self.player, 1))
    #         if self.options.enable_ngp:
    #             self.add_location_rule("[Chariot] Executioner's Chariot drop in NG+", lambda state: state.has("Estus Flask Shard", self.player, 2) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_location_rule("[Chariot] Above the stairs leading to the bonfire", lambda state: state.has("Estus Flask Shard", self.player, 2) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Earthen Peak", "Iron Keep", lambda state: state.has("Estus Flask Shard", self.player, 5) and state.has("Sublime Bone Dust", self.player, 2))
    #         #The Rotten Route
    #         self.add_connection_rule("Majula", "Grave of Saints", lambda state: state.has("Estus Flask Shard", self.player, 2) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Grave of Saints", "The Gutter", lambda state: state.has("Estus Flask Shard", self.player, 3) and state.has("Sublime Bone Dust", self.player, 2))
    #         #Duke's Dear Freja Route(especially the Royal Rat Authority)
    #         self.add_connection_rule("Shaded Woods", "Doors of Pharros", lambda state: state.has("Estus Flask Shard", self.player, 3) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Doors of Pharros", "Brightstone Cove", lambda state: state.has("Estus Flask Shard", self.player, 5) and state.has("Sublime Bone Dust", self.player, 2))
    #         #Late game
    #         self.add_connection_rule("Shaded Woods", "Drangleic Castle", lambda state: state.has("Estus Flask Shard", self.player, 8) and state.has("Sublime Bone Dust", self.player, 3))
    #         self.add_connection_rule("Shaded Woods", "Aldia's Keep", lambda state: state.has("Estus Flask Shard", self.player, 8) and state.has("Sublime Bone Dust", self.player, 3))
    #         if self.options.sunken_king_dlc:
    #             self.add_connection_rule("The Gutter", "Shulva", lambda state: state.has("Estus Flask Shard", self.player, 9) and state.has("Sublime Bone Dust", self.player, 4))
    #         if self.options.old_iron_king_dlc:
    #             self.add_connection_rule("Iron Keep", "Brume Tower", lambda state: state.has("Estus Flask Shard", self.player, 9) and state.has("Sublime Bone Dust", self.player, 4))
    #         if self.options.ivory_king_dlc:
    #             self.add_connection_rule("Drangleic Castle", "Eleum Loyce", lambda state: state.has("Estus Flask Shard", self.player, 9) and state.has("Sublime Bone Dust", self.player, 4))
        
    #     if self.options.combat_logic == "hard":
        
    #         #Lost Sinner Route
    #         self.add_connection_rule("FOFG - Soldier Key", "Lost Bastille - FOFG", lambda state: state.has("Estus Flask Shard", self.player, 1) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("No-man's Wharf", "Lost Bastille - Wharf", lambda state: state.has("Estus Flask Shard", self.player, 1) and state.has("Sublime Bone Dust", self.player, 1))
    #         self.add_connection_rule("Late Lost Bastille", "Sinners' Rise", lambda state: state.has("Estus Flask Shard", self.player, 3))
    #         #The Rotten Route
    #         self.add_connection_rule("Majula", "Grave of Saints", lambda state: state.has("Estus Flask Shard", self.player, 3) and state.has("Sublime Bone Dust", self.player, 1))
    #         if self.options.sunken_king_dlc:
    #             self.add_connection_rule("The Gutter", "Shulva", lambda state: state.has("Estus Flask Shard", self.player, 6) and state.has("Sublime Bone Dust", self.player, 3))
    #         if self.options.old_iron_king_dlc:
    #             self.add_connection_rule("Iron Keep", "Brume Tower", lambda state: state.has("Estus Flask Shard", self.player, 6) and state.has("Sublime Bone Dust", self.player, 3))
    #         if self.options.ivory_king_dlc:
    #             self.add_connection_rule("Drangleic Castle", "Eleum Loyce", lambda state: state.has("Estus Flask Shard", self.player, 6) and state.has("Sublime Bone Dust", self.player, 3))
        
    #     if self.options.combat_logic == "disabled": return

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
