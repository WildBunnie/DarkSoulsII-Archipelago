import string
import random

from worlds.AutoWorld import World
from worlds.generic.Rules import set_rule
from BaseClasses import Item, ItemClassification, Location, Region
from .Items import item_list, progression_items
from .Locations import location_table, dlc_regions
from .Options import DS2Options

class DS2Location(Location):
    game: str = "Dark Souls II"
    default_item: str

    def __init__(self, player, name, code, parent_region, default_item):
        self.default_item = default_item
        super(DS2Location, self).__init__(
            player, name, code, parent_region
        )

class DS2Item(Item):
    game: str = "Dark Souls II"

class DS2World(World):

    game = "Dark Souls II"

    options_dataclass = DS2Options
    options: DS2Options

    item_name_to_id = {name: code for (code, name) in item_list}
    location_name_to_id = {name: code for region in location_table.keys() for code, name, _ in location_table[region]}

    def create_regions(self):

        regions = {}

        menu_region = self.create_region("Menu")
        self.multiworld.regions.append(menu_region)
        regions["Menu"] = menu_region
    
        for region_name in location_table:
            if region_name in dlc_regions and not self.options.enable_dlcs: continue
            region = self.create_region(region_name)
            for _, location_name, default_item in location_table[region_name]:
                location = self.create_location(location_name, region, default_item)
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
        regions["Drangleic Castle"].connect(regions["Shrine of Amana"])
        regions["Shrine of Amana"].connect(regions["Undead Crypt"])
        
        regions["Aldia's Keep"].connect(regions["Dragon Aerie"])

        if self.options.enable_dlcs:
            regions["Shaded Woods"].connect(regions["Eleum Loyce"])
            regions["Iron Keep"].connect(regions["Brume Tower"])
            regions["The Gutter"].connect(regions["Shulva"])

    def create_region(self, name):
        return Region(name, self.player, self.multiworld)

    def create_location(self, name, region, default_item):
        return DS2Location(self.player, name, self.location_name_to_id[name], region, default_item)

    def create_items(self):
        pool = []
        
        final_item = self.create_item("Soul of Nashandra")
        self.multiworld.get_location("[Drangleic] Nashandra drop", self.player).place_locked_item(final_item)

        for location in self.multiworld.get_unfilled_locations(self.player):
            if not location.default_item: continue
            item = self.create_item(location.default_item)
            pool.append(item)

        self.multiworld.itempool += pool

    def create_item(self, name: str) -> DS2Item:
        code = self.item_name_to_id[name]
        classification = ItemClassification.progression if name in progression_items else ItemClassification.filler
        return DS2Item(name, classification, code, self.player)

    def set_rules(self):
        self.set_connection_rule("Path to Shaded Woods", "Shaded Woods", lambda state: state.has("Fragrant Branch of Yore", self.player))
        self.set_connection_rule("Forest of Fallen Giants", "Lost Bastille", lambda state: state.has("Soldier Key", self.player))
        self.set_connection_rule("Shaded Woods", "Aldia's Keep", lambda state: state.has("King's Ring", self.player))
        self.set_connection_rule("Forest of Fallen Giants", "Giant's Memory", lambda state: state.has("King's Ring", self.player) and state.has("Ashen Mist Heart", self.player))
        self.set_connection_rule("Drangleic Castle", "Throne of Want", lambda state: state.has("King's Ring", self.player))
        
        set_rule(self.multiworld.get_location("[Drangleic] Nashandra drop", self.player), lambda state: state.has("Giant's Kinship", self.player))

        self.multiworld.completion_condition[self.player] = lambda state: state.has("Soul of Nashandra", self.player)

        # from Utils import visualize_regions
        # visualize_regions(self.multiworld.get_region("Menu", self.player), "my_world.puml")

    def set_connection_rule(self, fromRegion, toRegion, state):
        set_rule(self.multiworld.get_entrance(f"{fromRegion} -> {toRegion}", self.player), state)

    def fill_slot_data(self) -> dict:
        return self.options.as_dict("death_link")