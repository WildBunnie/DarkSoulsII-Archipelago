import string
import random

from worlds.AutoWorld import World
from BaseClasses import Item, ItemClassification, Location, Region
from .Items import item_list, progression_items
from .Locations import location_table
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
            region = self.create_region(region_name)
            for _, location_name, default_item in location_table[region_name]:
                location = self.create_location(location_name, region, default_item)
                region.locations.append(location)
            regions[region_name] = region
            self.multiworld.regions.append(region)
        
        regions["Menu"].connect(regions["Things Betwixt"])
        regions["Things Betwixt"].connect(regions["Majula"])
        regions["Majula"].connect(regions["Forest of Fallen Giants"])

    def create_region(self, name):
        return Region(name, self.player, self.multiworld)

    def create_location(self, name, region, default_item):
        return DS2Location(self.player, name, self.location_name_to_id[name], region, default_item)

    def create_items(self):
        pool = []
        
        for location in self.multiworld.get_locations(self.player):
            item = self.create_item(location.default_item)
            pool.append(item)

        self.multiworld.itempool += pool

    def create_item(self, name: str) -> DS2Item:
        code = self.item_name_to_id[name]
        classification = ItemClassification.progression if name in progression_items else ItemClassification.filler
        return DS2Item(name, classification, code, self.player)