from .Enums import DLC

from dataclasses import dataclass
from typing import Optional, List, Dict

@dataclass
class RegionData:
    name: str
    """The Archipelago name for this region."""

    connections: List[str]
    """The names of all the regions directly connected to this region."""

    dlc: Optional[DLC] = None
    """The DLC this region belongs to, if applicable."""

region_list: List[RegionData] = [
    RegionData("Menu", ["Things Betwixt"]),
    RegionData("Things Betwixt", ["Majula"]),
    RegionData("Majula", ["Forest of Fallen Giants", "Heides Tower of Flame", "Huntsmans Copse", "The Pit", "Shaded Woods"]),

    RegionData("Forest of Fallen Giants", ["Forest of Fallen Giants - Soldier Key", "Forest of Fallen Giants - Salamander Pit", "Memory of Vammar"]),
    RegionData("Forest of Fallen Giants - Soldier Key", ["Memory of Orro", "Memory of Jeigh", "The Lost Bastille - FOFG"]),
    RegionData("Forest of Fallen Giants - Salamander Pit", []),

    RegionData("Memory of Vammar", []),
    RegionData("Memory of Orro", []),
    RegionData("Memory of Jeigh", []),

    RegionData("Heides Tower of Flame", ["No Mans Wharf"]),
    RegionData("Heides Tower of Flame", ["Cathedral of Blue"]),
    RegionData("Cathedral of Blue", []),
    RegionData("No Mans Wharf", ["The Lost Bastille - Wharf"]),

    RegionData("The Lost Bastille - FOFG", ["The Lost Bastille"]),
    RegionData("The Lost Bastille - Wharf", ["The Lost Bastille", "The Lost Bastille - After Key"]),
    RegionData("The Lost Bastille", ["The Lost Bastille - After Statue"]),
    RegionData("The Lost Bastille - After Key", ["The Lost Bastille - Late"]),
    RegionData("The Lost Bastille - After Statue", ["Belfry Luna", "The Lost Bastille - After Key", "The Lost Bastille - Late"]),
    RegionData("The Lost Bastille - Late", ["Sinners Rise"]),
    RegionData("Belfry Luna", []),
    RegionData("Sinners Rise", []),

    RegionData("Huntsmans Copse", ["Harvest Valley", "Undead Purgatory"]),
    RegionData("Undead Purgatory", []),
    RegionData("Harvest Valley", ["Earthen Peak"]),
    RegionData("Earthen Peak", ["Iron Keep"]),
    RegionData("Iron Keep", ["Belfry Sol", "Brume Tower"]),
    RegionData("Belfry Sol", []),

    RegionData("The Pit", ["Grave of Saints", "The Gutter"]),
    RegionData("Grave of Saints", []),
    RegionData("The Gutter", ["Black Gulch"]),
    RegionData("Black Gulch", ["Shulva Sanctum City"]),

    RegionData("Shaded Woods", ["Drangleic Castle", "Doors of Pharros", "Aldias Keep"]),
    RegionData("Doors of Pharros", ["Tseldora"]),
    RegionData("Tseldora", []),

    RegionData("Drangleic Castle", ["Shrine of Amana", "Throne of Want", "Eleum Loyce"]),
    RegionData("Shrine of Amana", ["Undead Crypt"]),
    RegionData("Undead Crypt", []),
    RegionData("Throne of Want", []),

    RegionData("Aldias Keep", ["Dragon Aerie"]),
    RegionData("Dragon Aerie", ["Dragon Shrine"]),
    RegionData("Dragon Shrine", []),

    RegionData("Shulva Sanctum City", [], dlc=DLC.SUNKEN_KING),
    RegionData("Dragons Sanctum", [], dlc=DLC.SUNKEN_KING),
    RegionData("Cave of The Dead", [], dlc=DLC.SUNKEN_KING),

    RegionData("Brume Tower", [], dlc=DLC.OLD_IRON_KING),
    RegionData("Brume Tower - Scepter", [], dlc=DLC.OLD_IRON_KING),
    RegionData("Iron Passage", [], dlc=DLC.OLD_IRON_KING),
    RegionData("Memory of The Old Iron King", [], dlc=DLC.OLD_IRON_KING),

    RegionData("Eleum Loyce", ["Frigid Outskirts"], dlc=DLC.IVORY_KING),
    RegionData("Frigid Outskirts", [], dlc=DLC.IVORY_KING),
]

region_map: Dict[str, RegionData] = {}
for region_data in region_list:
    region_map[region_data.name] = region_data

for region_data in region_map.values():
    if not region_data.connections: continue
    for connection in region_data.connections:
        assert connection in region_map.keys(), f"Invalid region: {connection} ({region_data.name} -> {connection})"