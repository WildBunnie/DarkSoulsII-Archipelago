import string
import random

from worlds.AutoWorld import World
from worlds.generic.Rules import set_rule, add_item_rule
from BaseClasses import Item, ItemClassification, Location, Region, LocationProgressType
from .Items import item_list, progression_items, repetable_categories, group_table, ItemCategory, DLC
from .Locations import location_table, location_name_groups
from .Options import DS2Options
from typing import Optional

class DS2Location(Location):
    game: str = "Dark Souls II"
    default_items: list[str]
    shop: bool = False

    def __init__(self, player, name, code, parent_region, default_items, shop):
        self.default_items = default_items
        self.shop = shop
        super(DS2Location, self).__init__(
            player, name, code, parent_region
        )

class DS2Item(Item):
    game: str = "Dark Souls II"
    category: ItemCategory

    def __init__(self, name, classification, code, player, category):
        self.category = category
        super(DS2Item, self).__init__(
            name, classification, code, player
        )

class DS2World(World):

    game = "Dark Souls II"

    options_dataclass = DS2Options
    options: DS2Options

    item_name_to_id = {item_data.name: item_data.code for item_data in item_list}
    location_name_to_id = {location_data.name: location_data.code for region in location_table.keys() for location_data in location_table[region] if location_data.code != None}
    item_name_groups = group_table
    location_name_groups = location_name_groups

    def generate_early(self) -> None:
        if self.options.early_blacksmith == "early_global":
            self.multiworld.early_items[self.player]["Lenigrast's Key"] = 1
        elif self.options.early_blacksmith == "early_local":
            self.multiworld.local_early_items[self.player]["Lenigrast's Key"] = 1

    def create_regions(self):

        regions = {}

        menu_region = self.create_region("Menu")
        self.multiworld.regions.append(menu_region)
        regions["Menu"] = menu_region
    
        for region_name in location_table:
            if region_name == "Shulva" and not self.options.sunken_king_dlc: continue
            if region_name == "Brume Tower" and not self.options.old_iron_king_dlc: continue
            if region_name == "Eleum Loyce" and not self.options.ivory_king_dlc: continue
            region = self.create_region(region_name)
            for location_data in location_table[region_name]:
                if location_data.ngp and not self.options.enable_ngp: continue
                if location_data.sotfs and not self.options.game_version == "sotfs": continue
                if location_data.vanilla and not self.options.game_version == "vanilla": continue

                if location_data.code == None: # event
                    location = DS2Location(self.player, location_data.name, None, region, None, False)
                else:
                    location = self.create_location(location_data.name, region, location_data.default_items, location_data.shop, location_data.skip)
                region.locations.append(location)
            regions[region_name] = region
            self.multiworld.regions.append(region)
        
        regions["Menu"].connect(regions["Things Betwixt"])

        regions["Things Betwixt"].connect(regions["Majula"])

        regions["Majula"].connect(regions["Forest of Fallen Giants"])
        regions["Majula"].connect(regions["Shaded Woods"])
        regions["Majula"].connect(regions["Heide's Tower of Flame"])
        regions["Majula"].connect(regions["Huntman's Copse"])
        regions["Majula"].connect(regions["Grave of Saints"])

        regions["Grave of Saints"].connect(regions["The Gutter"])
        regions["The Gutter"].connect(regions["Dark Chasm of Old"])

        regions["Forest of Fallen Giants"].connect(regions["Memory of Jeigh"])
        regions["Forest of Fallen Giants"].connect(regions["Memory of Vammar"])
        regions["Forest of Fallen Giants"].connect(regions["Memory of Orro"])
        regions["Forest of Fallen Giants"].connect(regions["Lost Bastille - FOFG"])

        regions["Heide's Tower of Flame"].connect(regions["No-man's Wharf"])
        regions["No-man's Wharf"].connect(regions["Lost Bastille - Wharf"])
        
        regions["Lost Bastille - FOFG"].connect(regions["Early Lost Bastille"])
        regions["Lost Bastille - Wharf"].connect(regions["Early Lost Bastille"])
        regions["Lost Bastille - Wharf"].connect(regions["Lost Bastille - After Key"])
        regions["Early Lost Bastille"].connect(regions["Lost Bastille - After Statue"])
        regions["Lost Bastille - After Statue"].connect(regions["Belfry Luna"])
        regions["Lost Bastille - After Statue"].connect(regions["Lost Bastille - After Key"])
        regions["Lost Bastille - After Statue"].connect(regions["Late Lost Bastille"])
        regions["Lost Bastille - After Key"].connect(regions["Late Lost Bastille"])
        regions["Late Lost Bastille"].connect(regions["Sinners' Rise"])
        
        regions["Huntman's Copse"].connect(regions["Earthen Peak"])
        regions["Earthen Peak"].connect(regions["Iron Keep"])
        regions["Iron Keep"].connect(regions["Belfry Sol"])

        regions["Shaded Woods"].connect(regions["Drangleic Castle"])
        regions["Shaded Woods"].connect(regions["Doors of Pharros"])
        regions["Shaded Woods"].connect(regions["Aldia's Keep"])
        regions["Shaded Woods"].connect(regions["Dark Chasm of Old"])

        regions["Doors of Pharros"].connect(regions["Brightstone Cove"])

        regions["Drangleic Castle"].connect(regions["Throne of Want"])
        regions["Drangleic Castle"].connect(regions["Dark Chasm of Old"])
        regions["Drangleic Castle"].connect(regions["King's Passage"])
        regions["King's Passage"].connect(regions["Shrine of Amana"])
        regions["Shrine of Amana"].connect(regions["Undead Crypt"])
        
        regions["Aldia's Keep"].connect(regions["Dragon Aerie"])

        if self.options.sunken_king_dlc:
            regions["The Gutter"].connect(regions["Shulva"])
        if self.options.old_iron_king_dlc:
            regions["Iron Keep"].connect(regions["Brume Tower"])
        if self.options.ivory_king_dlc:
            regions["Shaded Woods"].connect(regions["Eleum Loyce"])

    def create_region(self, name):
        return Region(name, self.player, self.multiworld)

    def create_location(self, name, region, default_items, shop=False, skip=False):
        location = DS2Location(self.player, name, self.location_name_to_id[name], region, default_items, shop)
        if skip: location.progress_type = LocationProgressType.EXCLUDED
        return location

    def create_items(self):
        pool : list[DS2Item] = []

        events = [location for region in location_table.keys() for location in location_table[region] if location.event == True]
        for event in events:
            event_item = DS2Item(event.name, ItemClassification.progression, None, self.player, None)
            self.multiworld.get_location(event.name, self.player).place_locked_item(event_item)
        
        # set the giant's kinship at the original location
        # because killing the giant lord is necessary to kill nashandra
        self.multiworld.get_location("[MemoryJeigh] Giant Lord drop", self.player).place_locked_item(self.create_item("Giant's Kinship"))

        max_pool_size = len(self.multiworld.get_unfilled_locations(self.player))

        statues = [item for item in item_list if item.category == ItemCategory.STATUE]
        if self.options.game_version == "vanilla":
            statues = [item for item in statues if not item.sotfs]

        # fill pool with all the default items from each location
        items_in_pool = [item.name for item in pool]
        for location in self.multiworld.get_unfilled_locations(self.player):
            for item_name in location.default_items:

                if item_name == "Fragrant Branch of Yore":
                    if len(statues) == 0: continue
                    item_data = statues.pop()
                elif item_name == "Pharros' Lockstone":
                    if "Master Lockstone" in items_in_pool: continue
                    item_data = next((item for item in item_list if item.name == "Master Lockstone"), None)
                else:
                    item_data = next((item for item in item_list if item.name == item_name), None)
                    assert item_data, f"location's default item not in item list '{item_name}'"

                # skip unwanted items
                if item_data.skip: continue
                # dont allow duplicates
                if item_data.category not in repetable_categories and item_data.name in items_in_pool: continue
                # skip sotfs items if we are not in sotfs
                if item_data.sotfs and not self.options.game_version == "sotfs": continue

                item = self.create_item(item_data.name, item_data.category)
                items_in_pool.append(item_data.name)
                pool.append(item)

        diff = len(pool) - max_pool_size

        # remove filler items so pool is not overfilled
        if diff > 0:
            while diff != 0:
                item = random.choice(pool)
                if item.category in repetable_categories:
                    pool.remove(item)
                    diff -= 1
        # fill pool with filler items
        elif diff < 0:
            filler_items = [item for item in item_list if item.category in repetable_categories and not item.skip and not item.sotfs and self.is_dlc_allowed(item.dlc)]
            for _ in range(abs(diff)):
                item_data = random.choice(filler_items)
                item = self.create_item(item_data.name, item_data.category)
                pool.append(item)

        assert len(pool) == max_pool_size, "item pool is under-filled or over-filled"

        self.multiworld.itempool += pool

    def create_item(self, name: str, category=None) -> DS2Item:
        code = self.item_name_to_id[name]
        classification = ItemClassification.progression if name in progression_items or category==ItemCategory.STATUE else ItemClassification.filler
        return DS2Item(name, classification, code, self.player, category)

    def is_dlc_allowed(dlc, options):
        dlc_conditions = {
            DLC.SUNKEN_KING: options.sunken_king_dlc,
            DLC.OLD_IRON_KING: options.old_iron_king_dlc,
            DLC.IVORY_KING: options.ivory_king_dlc
        }

        if dlc == DLC.ALL:
            return any(dlc_conditions.values())
        
        return dlc_conditions.get(dlc, False)

    def set_rules(self):

        for location in self.multiworld.get_locations(self.player):
            if location.shop:
                add_item_rule(location, lambda item: 
                              item.player != self.player or
                              item.category not in [ItemCategory.AMMO, ItemCategory.CONSUMABLE, ItemCategory.STATUE])

        self.set_shop_rules()

        # EVENTS
        self.set_location_rule("Rotate the Majula Rotunda", lambda state: state.has("Rotunda Lockstone", self.player))
        self.set_location_rule("Open Shrine of Winter", lambda state: 
            (state.has("Defeat the Rotten", self.player) and
             state.has("Defeat the Lost Sinner", self.player) and
             state.has("Defeat the Old Iron King", self.player) and
             state.has("Defeat the Duke's Dear Freja", self.player)))

        # LOCATIONS
        ## MAJULA
        self.set_location_rule("[Majula] Wooden chest in Lenigrast's workshop", lambda state: state.has("Lenigrast's Key", self.player))
        self.set_location_rule("[Majula] Library room in Cale's house", lambda state: state.has("House Key", self.player))
        self.set_location_rule("[Majula] Corpse in Cale's house basement", lambda state: state.has("House Key", self.player))
        self.set_location_rule("[Majula] Metal chest in Cale's house basement", lambda state: state.has("House Key", self.player))
        self.set_location_rule("[Majula] Wooden chest on the attic of Majula mansion", lambda state: state.has("House Key", self.player))
        ## PURSUER
        self.set_location_rule("[FOFG] Just before pursuer arena", lambda state: state.has("Soldier Key", self.player))
        if self.options.enable_ngp:
            self.set_location_rule("[FOFG] Just before pursuer arena in NG+", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("Defeat the Pursuer (in the proper arena)", lambda state: state.has("Soldier Key", self.player))
        ## Soldier's Rest
        self.set_location_rule("[FOFG] In the beginning of the dark skeleton tunnel", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] At the end of the dark skeleton tunnel", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] In the small stone house near Soldier's Rest bonfire", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] Wooden chest near Soldier's Rest bonfire", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] First corpse at rooftop near Soldier's Rest bonfire", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] Wooden chest near Soldier's Rest bonfire", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] Next to portcullis near Soldier's rest bonfire", lambda state: state.has("Soldier Key", self.player))
        if self.options.game_version == "sotfs":
            self.set_location_rule("[FOFG] Second corpse at rooftop near Soldier's Rest bonfire", lambda state: state.has("Soldier Key", self.player))
            self.set_location_rule("[FOFG] Third corpse at rooftop near Soldier's Rest bonfire", lambda state: state.has("Soldier Key", self.player))
            self.set_location_rule("[FOFG] Fourth corpse at rooftop near Soldier's Rest bonfire", lambda state: state.has("Soldier Key", self.player))
        ## Before Kings Door
        self.set_location_rule("[FOFG] Wooden chest in a side corridor on the way to the king's door", lambda state: state.has("Soldier Key", self.player))
        if self.options.game_version == "sotfs":
            self.set_location_rule("[FOFG] Wooden chest next to king's door", lambda state: state.has("Soldier Key", self.player))
        ## UPPER FLOOR CARDINAL TOWER
        self.set_location_rule("[FOFG] Wooden chest in upper floor of cardinal tower", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] Metal chest in upper floor of cardinal tower", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] Drop onto tree branch from upper floor of Cardinal Tower", lambda state: state.has("Soldier Key", self.player))
        self.set_location_rule("[FOFG] Behind the wagon at Cardinal Tower upper floor", lambda state: state.has("Soldier Key", self.player))
        if self.options.game_version == "sotfs":
            self.set_location_rule("[FOFG] Behind a table at cardinal tower upper floor", lambda state: state.has("Soldier Key", self.player))
        ## LOWER FIRE AREA
        self.set_location_rule("[FOFG] First corpse in the lower fire area", lambda state: state.has("Iron Key", self.player))
        self.set_location_rule("[FOFG] Second corpse in the lower fire area", lambda state: state.has("Iron Key", self.player))
        ## TSELDORA DEN
        self.set_location_rule("[Tseldora] Metal chest in Tseldora den", lambda state: state.has("Tseldora Den Key", self.player))
        self.set_location_rule("[Tseldora] Wooden chest in Tseldora den", lambda state: state.has("Tseldora Den Key", self.player))
        self.set_location_rule("[Tseldora] Metal chest behind locked door in pickaxe room", lambda state: state.has("Brightstone Key", self.player))
        ## FORGOTTEN KEY
        self.set_location_rule("[Pit] First metal chest behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
        self.set_location_rule("[Pit] Third metal chest behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
        self.set_location_rule("[Pit] Second metal chest behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
        if self.options.game_version == "sotfs": 
            self.set_location_rule("[Pit] Corpse behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
        self.set_location_rule("[Gutter] Urn behind the forgotten door", lambda state: state.has("Forgotten Key", self.player))
        ## BASTILLE KEY
        self.set_location_rule("[Bastille] In a cell next to Straid's cell", lambda state: state.has("Bastille Key", self.player))
        self.set_location_rule("[SinnersRise] In locked cell left side upper level", lambda state: state.has("Bastille Key", self.player))
        self.set_location_rule("[SinnersRise] In right side oil-sconce room just before the Sinner", lambda state: state.has("Bastille Key", self.player))
        if self.options.enable_ngp:
            self.set_location_rule("[Bastille] In a cell next to Straid's cell in NG+", lambda state: state.has("Bastille Key", self.player))
    
        self.set_location_rule("[ShadedWoods] Gift from Manscorpion Tark after defeating Najka", lambda state: state.has("Ring of Whispers", self.player))

        #STATUES
        if self.options.game_version == "sotfs":
            self.set_location_rule("[Betwixt] In the basilisk pit", lambda state: state.has("Unpetrify Statue in Things Betwixt", self.player))
            self.set_location_rule("[Heides] Metal chest behind petrified hollow after Dragonrider", lambda state: state.has("Unpetrify Statue in Heide's Tower of Flame", self.player))
            self.set_location_rule("[Heides] On railing behind petrified hollow", lambda state: state.has("Unpetrify Statue in Heide's Tower of Flame", self.player))
            self.set_location_rule("Defeat the Rotten", lambda state: state.has("Unpetrify Statue in Black Gulch", self.player))
            self.set_location_rule("[Gulch] Urn next to the second bonfire", lambda state: state.has("Unpetrify Statue in Black Gulch", self.player))
            self.set_location_rule("[ShadedWoods] Metal chest blocked by petrified statue", lambda state: state.has("Unpetrify Statue Blocking the Chest in Shaded Ruins", self.player))
            self.set_location_rule("[ShadedWoods] Drop from Petrified Lion Warrior next to Golden Lion Warrior", lambda state: state.has("Unpetrify Warlock Mask Statue in Shaded Ruins", self.player))
            self.set_location_rule("[AldiasKeep] Drop from Petrified Undead Traveller just before Giant Basilisk", lambda state: state.has("Unpetrify Left Cage Statue in Aldia's Keep", self.player))
            self.set_location_rule("[AldiasKeep] Drop from Centre petrified Undead Traveller just before Giant Basilisk", lambda state: state.has("Unpetrify Right Cage Statue in Aldia's Keep", self.player))
        self.set_location_rule("[ShadedWoods] Metal chest in room blocked by petrified statue", lambda state: state.has("Unpetrify Lion Mage Set Statue in Shaded Ruins", self.player))
        self.set_location_rule("[ShadedWoods] Drop from the petrified lion warrior by the tree bridge", lambda state: state.has("Unpetrify Fang Key Statue in Shaded Ruins", self.player))
        self.set_location_rule("[AldiasKeep] Drop from Petrified Ogre blocking stairway near Bone Dragon", lambda state: state.has("Unpetrify Cyclops Statue in Aldia's Keep", self.player))

        # lockstones
        self.set_location_rule("[FOFG] First metal chest behind Pharros' contraption under the ballista-trap", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[FOFG] Second metal chest behind Pharros' contraption under the ballista-trap", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[Bastille] Wooden chest behind Pharros' contraption in Pharros/elevator room", lambda state: state.has("Master Lockstone", self.player))
        if self.options.game_version == "vanilla":
            self.set_location_rule("[Bastille] Metal chest next to elevator in Pharros/elevator room", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[EarthernPeak] Metal chest behind Pharros contraption in the lowest level next to Lucatiel", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[DragonShrine] Metal chest behind the Pharros contraption under the staircase", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[Pharros] Wooden chest in room after using top Pharros contraption and dropping down near the toxic rats", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[Pharros] Trapped wooden chest behind (floor) Pharros contraption in the upper level", lambda state: state.has("Master Lockstone", self.player))
        if self.options.enable_ngp:
            self.set_location_rule("[Pharros] Trapped wooden chest behind (floor) Pharros contraption in the upper level in NG+", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[Pharros] Metal chest behind three-part pharros door in the lower level", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[MemoryOrro] Trapped wooden chest behind a Pharros' contraption on the second floor", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[MemoryOrro] Metal chest behind a Pharros contraption and an illusory wall on the second floor", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[MemoryOrro] Metal chest behind a Pharros contraption and an illusory wall on the second floor (2)", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[Amana] Metal chest behind a pharros contraption near the crumbled ruins bonfire", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[Crypt] Metal chest behind a illusory wall and a Pharros contraption from the third graveyard room", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[GraveOfSaints] 1st floor on other side of the drawbridges", lambda state: state.has("Master Lockstone", self.player))
        self.set_location_rule("[GraveOfSaints] 2nd floor on other side of the drawbridges", lambda state: state.has("Master Lockstone", self.player))

        # CONNECTIONS
        if self.options.game_version == "sotfs":
            if self.options.sunken_king_dlc:
                self.set_connection_rule("The Gutter", "Shulva", lambda state: state.has("Dragon Talon", self.player))
            if self.options.old_iron_king_dlc:
                self.set_connection_rule("Iron Keep", "Brume Tower", lambda state: state.has("Heavy Iron Key", self.player))
            if self.options.ivory_king_dlc:
                self.set_connection_rule("Shaded Woods", "Eleum Loyce", lambda state: 
                                            state.has("Frozen Flower", self.player) and 
                                            state.has("Open Shrine of Winter", self.player))

        self.set_connection_rule("Majula", "Huntman's Copse", lambda state: state.has("Rotate the Majula Rotunda", self.player))
        self.set_connection_rule("Majula", "Grave of Saints", lambda state: state.has("Silvercat Ring", self.player) or state.has("Flying Feline Boots", self.player))
        self.set_connection_rule("Majula", "Shaded Woods", lambda state: state.has("Unpetrify Rosabeth of Melfia", self.player))
        self.set_connection_rule("Forest of Fallen Giants", "Lost Bastille - FOFG", lambda state: state.has("Soldier Key", self.player))
        self.set_connection_rule("Shaded Woods", "Aldia's Keep", lambda state: state.has("King's Ring", self.player))
        self.set_connection_rule("Shaded Woods", "Drangleic Castle", lambda state: state.has("Open Shrine of Winter", self.player))
        self.set_connection_rule("Drangleic Castle", "King's Passage", lambda state: state.has("Key to King's Passage", self.player))
        self.set_connection_rule("Forest of Fallen Giants", "Memory of Vammar", lambda state: state.has("Ashen Mist Heart", self.player))
        self.set_connection_rule("Forest of Fallen Giants", "Memory of Orro", lambda state: 
                                    state.has("Ashen Mist Heart", self.player) and
                                    state.has("Defeat the Pursuer (in the proper arena)", self.player))
        self.set_connection_rule("Forest of Fallen Giants", "Memory of Jeigh", lambda state: 
                                    state.has("King's Ring", self.player) and 
                                    state.has("Ashen Mist Heart", self.player) and 
                                    state.has("Soldier Key", self.player))
        self.set_connection_rule("Drangleic Castle", "Throne of Want", lambda state: state.has("King's Ring", self.player))
        self.set_connection_rule("Iron Keep", "Belfry Sol", lambda state: state.has("Master Lockstone", self.player))

        # LOST BASTILLE
        self.set_connection_rule("Lost Bastille - Wharf", "Lost Bastille - After Key", lambda state: state.has("Antiquated Key", self.player))
        self.set_connection_rule("Lost Bastille - After Statue", "Belfry Luna", lambda state: state.has("Master Lockstone", self.player))
        if self.options.game_version == "sotfs": 
            self.set_connection_rule("Early Lost Bastille", "Lost Bastille - After Statue", lambda state: state.has("Unpetrify Statue in Lost Bastille", self.player))
        elif self.options.game_version == "vanilla":
            self.set_connection_rule("Lost Bastille - After Key", "Late Lost Bastille", lambda state: state.has("Master Lockstone", self.player))
            
        set_rule(self.multiworld.get_location("Defeat Nashandra", self.player), lambda state: state.has("Giant's Kinship", self.player))

        self.multiworld.completion_condition[self.player] = lambda state: state.has("Defeat Nashandra", self.player)

        # from Utils import visualize_regions
        # visualize_regions(self.multiworld.get_region("Menu", self.player), "my_world.puml")

    def set_connection_rule(self, fromRegion, toRegion, state):
        set_rule(self.multiworld.get_entrance(f"{fromRegion} -> {toRegion}", self.player), state)

    def set_location_rule(self, name, state):
        set_rule(self.multiworld.get_location(name, self.player), state)

    def set_shop_rules(self):
        self.set_location_rule("[Sweet Shalquoir - Royal Rat Authority, Royal Rat Vanguard] Flying Feline Boots", lambda state: 
                               state.has("Defeat the Royal Rat Authority", self.player) and state.has("Defeat the Royal Rat Vanguard", self.player))
        
        self.set_location_rule("[Lonesome Gavlan - Harvest Valley] Ring of Giants", lambda state: state.has("Speak with Lonesome Gavlan in No-man's Wharf", self.player))

        for region in location_table:
            for location in location_table[region]:
                if "[Laddersmith Gilligan - Majula]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Defeat Mytha, the Baneful Queen", self.player))
                elif "[Rosabeth of Melfia]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Unpetrify Rosabeth of Melfia", self.player))
                elif "[Blacksmith Lenigrast]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Lenigrast's Key", self.player))
                elif "[Steady Hand McDuff]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Dull Ember", self.player))
                elif "[Lonesome Gavlan - Doors of Pharros]" in location_table:
                    self.set_location_rule(location.name, lambda state: state.has("Speak with Lonesome Gavlan in Harvest Valley", self.player))
                elif "Straid of Olaphis" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Unpetrify Straid of Olaphis", self.player))
                elif " - Shrine of Winter]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Open Shrine of Winter", self.player))
                elif " - Skeleton Lords]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Defeat the Skeleton Lords", self.player))
                elif " - Looking Glass Knight]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Defeat the Looking Glass Knight", self.player))
                elif " - Lost Sinner]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Defeat the Lost Sinner", self.player))
                elif " - Old Iron King]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Defeat the Old Iron King", self.player))
                elif " - Velstadt]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Defeat Velstadt", self.player))
                elif " - Smelter Demon]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Defeat the Smelter Demon", self.player))
                    
    def fill_slot_data(self) -> dict:
        return self.options.as_dict("death_link","game_version","no_weapon_req","no_spell_req","no_equip_load","infinite_lifegems","randomize_starting_loadout", "starting_weapon_requirement", "autoequip")
