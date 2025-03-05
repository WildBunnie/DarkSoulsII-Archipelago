import string
import random

from worlds.AutoWorld import World
from worlds.generic.Rules import set_rule, add_item_rule
from BaseClasses import Item, ItemClassification, Location, Region
from .Items import item_list, progression_items, repetable_categories, group_table
from .Locations import location_table, dlc_regions
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
    repeatable: bool

    def __init__(self, name, classification, code, player, repeatable):
        self.repeatable = repeatable
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

    def create_regions(self):

        regions = {}

        menu_region = self.create_region("Menu")
        self.multiworld.regions.append(menu_region)
        regions["Menu"] = menu_region
    
        for region_name in location_table:
            if region_name in dlc_regions and not self.options.enable_dlcs: continue
            region = self.create_region(region_name)
            for location_data in location_table[region_name]:
                if location_data.ngp and not self.options.enable_ngp: continue
                if location_data.sotfs and not self.options.game_version == "sotfs": continue
                if location_data.vanilla and not self.options.game_version == "vanilla": continue
                if location_data.code == None: # event
                    location = DS2Location(self.player, location_data.name, None, region, None, False)
                else:
                    location = self.create_location(location_data.name, region, location_data.default_items, location_data.shop)
                region.locations.append(location)
            regions[region_name] = region
            self.multiworld.regions.append(region)
        
        regions["Menu"].connect(regions["Things Betwixt"])

        regions["Things Betwixt"].connect(regions["Majula"])

        regions["Majula"].connect(regions["Forest of Fallen Giants"])
        regions["Majula"].connect(regions["Path to Shaded Woods"])
        regions["Majula"].connect(regions["Heide's Tower of Flame"])
        regions["Majula"].connect(regions["Huntman's Copse"])
        regions["Majula"].connect(regions["Grave of Saints"])

        regions["Grave of Saints"].connect(regions["The Gutter"])
        regions["The Gutter"].connect(regions["Chasm of the Abyss"])

        regions["Forest of Fallen Giants"].connect(regions["Giant's Memory"])
        regions["Forest of Fallen Giants"].connect(regions["Lost Bastille"])

        regions["Heide's Tower of Flame"].connect(regions["Unseen Path to Heide"])
        regions["Unseen Path to Heide"].connect(regions["No-man's Wharf"])
        regions["No-man's Wharf"].connect(regions["Lost Bastille"])

        regions["Huntman's Copse"].connect(regions["Earthen Peak"])
        regions["Earthen Peak"].connect(regions["Iron Keep"])

        regions["Path to Shaded Woods"].connect(regions["Shaded Woods"])
        regions["Shaded Woods"].connect(regions["Drangleic Castle"])
        regions["Shaded Woods"].connect(regions["Doors of Pharros"])
        regions["Shaded Woods"].connect(regions["Aldia's Keep"])
        regions["Shaded Woods"].connect(regions["Chasm of the Abyss"])

        regions["Doors of Pharros"].connect(regions["Brightstone Cove"])
        regions["Brightstone Cove"].connect(regions["Dragon Memories"])

        regions["Drangleic Castle"].connect(regions["Throne of Want"])
        regions["Drangleic Castle"].connect(regions["Chasm of the Abyss"])
        regions["Drangleic Castle"].connect(regions["King's Passage"])
        regions["King's Passage"].connect(regions["Shrine of Amana"])
        regions["Shrine of Amana"].connect(regions["Undead Crypt"])
        
        regions["Aldia's Keep"].connect(regions["Dragon Aerie"])

        if self.options.enable_dlcs:
            regions["Shaded Woods"].connect(regions["Eleum Loyce"])
            regions["Iron Keep"].connect(regions["Brume Tower"])
            regions["The Gutter"].connect(regions["Shulva"])

    def create_region(self, name):
        return Region(name, self.player, self.multiworld)

    def create_location(self, name, region, default_items, shop):
        return DS2Location(self.player, name, self.location_name_to_id[name], region, default_items, shop)

    def create_items(self):
        pool : list[DS2Item] = []

        events = [location for region in location_table.keys() for location in location_table[region] if location.event == True]
        for event in events:
            event_item = DS2Item(event.name, ItemClassification.progression, None, self.player, False)
            self.multiworld.get_location(event.name, self.player).place_locked_item(event_item)
        
        # set the giant's kinship at the original location
        # because killing the giant lord is necessary to kill nashandra
        self.multiworld.get_location("[MemoryJeigh] Giant Lord drop", self.player).place_locked_item(self.create_item("Giant's Kinship"))

        max_pool_size = len(self.multiworld.get_unfilled_locations(self.player))

        # initial attempt at filling the item pool with the default items
        items_in_pool = [item.name for item in pool]
        for location in self.multiworld.get_unfilled_locations(self.player):
            
            # check if location contains a progression item
            index = self.get_progression_item(location.default_items)

            # if the location contains a progression item
            # we add that item to the pool, otherwise pick one randomly
            if index != -1:
                item_name = location.default_items[index]
            else:
                item_name = random.choice(location.default_items)

            item_data = next((item for item in item_list if item.name == item_name), None)
            assert item_data, "location's default item not in item list"

            # skip sotfs items if we are not in sotfs
            if item_data.sotfs and not self.options.game_version == "sotfs": continue
            # skip unwanted items
            if item_data.skip: continue
            # dont allow duplicates
            if item_data.category not in repetable_categories and item_data.name in items_in_pool: continue

            item = self.create_item(item_data.name, item_data.category)
            items_in_pool.append(item_data.name)
            pool.append(item)

        # fill the rest of the pool with filler items
        filler_items = [item for item in item_list if item.category in repetable_categories and not item.skip and not item.sotfs]
        for _ in range(max_pool_size - len(pool)):
            item = self.create_item(random.choice(filler_items).name, item_data.category)
            pool.append(item)

        assert len(pool) == max_pool_size, "item pool is under-filled or over-filled"

        self.multiworld.itempool += pool

    def create_item(self, name: str, category=None) -> DS2Item:
        code = self.item_name_to_id[name]
        classification = ItemClassification.progression if name in progression_items else ItemClassification.filler
        repeatable = True if category in repetable_categories and category != None else False
        return DS2Item(name, classification, code, self.player, repeatable)

    # given a list, returns the index of the first progression item
    # if no progression item is found inside the list, returns -1
    def get_progression_item(self, list):
        for index,item in enumerate(list):
            if item in progression_items:
                return index
        return -1

    def set_rules(self):

        # make it so shops only have local, non repeatable items
        # this is because the implementation for showing items
        # from other worlds inside the shop is a bit lackluster
        for location in self.multiworld.get_locations(self.player):
            if location.shop:
                add_item_rule(location, lambda item: item.player == self.player)

        self.set_shop_rules()

        self.set_location_rule("Rotate the Majula Rotunda", lambda state: state.has("Rotunda Lockstone", self.player))
        self.set_location_rule("Unpetrify Rosabeth of Melfia", lambda state: state.has("Fragrant Branch of Yore", self.player))
        self.set_location_rule("Open Shrine of Winter", lambda state: 
            (state.has("Defeat the Rotten", self.player) and
             state.has("Defeat the Lost Sinner", self.player) and
             state.has("Defeat the Old Iron King", self.player) and
             state.has("Defeat the Duke's Dear Freja", self.player)))
        self.set_location_rule(self.multiworld.get_location("[IronKeep] Metal chest in lava at right side of the first big lava room", lambda state:
            (state.has_group("fire_res_ring", player, 1) or
             state.has_group("fire_res_pyro", player, 2) or
             state.has_group("fire_res_darkpyro", player, 2)))
        self.set_location_rule(self.multiworld.get_location("[IronKeep] Metal chest in lava in the room with changing platforms", lambda state: 
            (state.has_group("fire_res_ring", player, 1) or
             state.has_group("fire_res_pyro", player, 2) or
             state.has_group("fire_res_darkpyro", player, 2)))
        self.set_location_rule(self.multiworld.get_location("[IronKeep] On lava on the right of bridge by first bonfire", lambda state: 
            (state.has_group("fire_res_ring", player, 1) or
             state.has_group("fire_res_pyro", player, 2) or
             state.has_group("fire_res_darkpyro", player, 2)))
        self.set_location_rule(self.multiworld.get_location("[IronKeep] On lava next to the first bonfire", lambda state: 
            (state.has_group("fire_res_ring", player, 1) or
             state.has_group("fire_res_pyro", player, 2) or
             state.has_group("fire_res_darkpyro", player, 2)))
        self.set_location_rule(self.multiworld.get_location("[Drangleic] Looking Glass Knight drop", self.player), lambda state: state.has("Key to King's Passage"))
        self.set_location_rule(self.multiworld.get_location("[FOFG] First corpse in the lower fire area", self.player), lambda state: state.has("Iron Key"))
        self.set_location_rule(self.multiworld.get_location("[FOFG] Second corpse in the lower fire area", self.player), lambda state: state.has("Iron Key"))
        self.set_location_rule(self.multiworld.get_location("[Tseldora] Metal chest in Tseldora den", self.player), lambda state: state.has("Tseldora Den Key"))
        self.set_location_rule(self.multiworld.get_location("[Tseldora] Wooden chest in Tseldora den", self.player), lambda state: state.has("Tseldora Den Key"))
        self.set_location_rule(self.multiworld.get_location("[Tseldora] Metal chest behind locked door in pickaxe room", self.player), lambda state: state.has("Brightstone Key"))
        
        self.set_connection_rule("Majula", "Huntman's Copse", lambda state: state.has("Rotate the Majula Rotunda", self.player))
        self.set_connection_rule("Majula", "Grave of Saints", state.has("Silvercat Ring", self.player) or state.has("Flying Feline Boots", self.player))
        self.set_connection_rule("Path to Shaded Woods", "Shaded Woods", lambda state: state.has("Unpetrify Rosabeth of Melfia", self.player))
        self.set_connection_rule("Forest of Fallen Giants", "Lost Bastille", lambda state: state.has("Soldier Key", self.player))
        self.set_connection_rule("Shaded Woods", "Aldia's Keep", lambda state: state.has("King's Ring", self.player))
        self.set_connection_rule("Shaded Woods", "Drangleic Castle", lambda state: state.has("Open Shrine of Winter", self.player))
        self.set_connection_rule("Drangleic Castle", "King's Passage", lambda state: state.has("Key to King's Passage", self.player))
        self.set_connection_rule("Forest of Fallen Giants", "Giant's Memory", lambda state: state.has("King's Ring", self.player) and state.has("Ashen Mist Heart", self.player) and state.has("Soldier Key"))
        self.set_connection_rule("Drangleic Castle", "Throne of Want", lambda state: state.has("King's Ring", self.player))
        self.set_connection_rule("Drangleic Castle", "Shrine of Amana", lambda state: state.has("Key to King's Passage", self.player))
        self.set_connection_rule("Iron Keep", "Belfry Sol", lambda state: state.has("Pharros' Lockstone", self.player))
        self.set_connection_rule("Lost Bastille", "Belfry Luna", lambda state: state.has("Pharros' Lockstone", self.player))

        set_rule(self.multiworld.get_location("Defeat Nashandra", self.player), lambda state: state.has("Giant's Kinship", self.player))

        self.multiworld.completion_condition[self.player] = lambda state: state.has("Defeat Nashandra", self.player)

        # from Utils import visualize_regions
        # visualize_regions(self.multiworld.get_region("Menu", self.player), "my_world.puml")

    def set_connection_rule(self, fromRegion, toRegion, state):
        set_rule(self.multiworld.get_entrance(f"{fromRegion} -> {toRegion}", self.player), state)

    def set_location_rule(self, name, state):
        set_rule(self.multiworld.get_location(name, self.player), state)


    def set_shop_rules(self):
        self.set_location_rule("[Merchant Hag Melentia - Majula] Lifegem", lambda state: state.has("Defeat the Last Giant", self.player))
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
                elif "[Steady Hand McDuff - Dull Ember]" in location.name:
                    self.set_location_rule(location.name, lambda state: state.has("Dull Ember", self.player))
                elif "[Lonesome Gavlan - Doors of Pharros]" in location_table:
                    self.set_location_rule(location.name, lambda state: state.has("Speak with Lonesome Gavlan in Harvest Valley", self.player))
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
        return self.options.as_dict("death_link","game_version","no_weapon_req")
