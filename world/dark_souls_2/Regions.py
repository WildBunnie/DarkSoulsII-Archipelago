from .Enums import DLC

from dataclasses import dataclass
from typing import Optional, List, Dict

@dataclass
class RegionData:
    connections: Optional[List[str]]
    dlc: Optional[DLC] = None

region_connections: Dict[str, RegionData] = {
    "Menu": 
        RegionData(["Things Betwixt"]),

    "Things Betwixt": 
        RegionData(["Majula"]),

    "Majula": 
        RegionData([
            "Forest of Fallen Giants",
            "Heides Tower of Flame",
            "The Pit",
            "Huntsmans Copse",
            "Shaded Woods",
        ]),

    "Forest of Fallen Giants": 
        RegionData([
            "Memory of Vammar",
            "Memory of Orro",
            "Memory of Jeigh",
            "The Lost Bastille",
        ]),

    "Memory of Vammar": 
        RegionData(None),

    "Memory of Orro": 
        RegionData(None),

    "Memory of Jeigh": 
        RegionData(None),

    "Heides Tower of Flame": 
        RegionData([
            "No Mans Wharf",
            "Cathedral of Blue",
        ]),

    "Cathedral of Blue": 
        RegionData(None),

    "No Mans Wharf": 
        RegionData(["The Lost Bastille"]),

    "The Lost Bastille": 
        RegionData([
            "Sinners Rise",
            "Belfry Luna",
        ]),

    "Sinners Rise": 
        RegionData(None),

    "Belfry Luna": 
        RegionData(None),

    "The Pit": 
        RegionData([
            "Grave of Saints",
            "The Gutter",
        ]),

    "Grave of Saints": 
        RegionData(None),

    "The Gutter": 
        RegionData(["Black Gulch"]),

    "Black Gulch": 
        RegionData(["Shulva Sanctum City"]),

    "Shulva Sanctum City": 
        RegionData(["Cave of The Dead"], dlc=DLC.SUNKEN_KING),

    "Cave of The Dead": 
        RegionData(None, dlc=DLC.SUNKEN_KING),

    "Huntsmans Copse": 
        RegionData([
            "Undead Purgatory",
            "Harvest Valley",
        ]),

    "Undead Purgatory": 
        RegionData(None),

    "Harvest Valley": 
        RegionData(["Earthen Peak"]),

    "Earthen Peak": 
        RegionData(["Iron Keep"]),

    "Iron Keep": 
        RegionData([
            "Belfry Sol",
            "Brume Tower",
        ]),

    "Belfry Sol": 
        RegionData(None),

    "Brume Tower": 
        RegionData([
            "Iron Passage",
            "Memory of The Old Iron King",
        ], dlc=DLC.OLD_IRON_KING),

    "Iron Passage": 
        RegionData(None, dlc=DLC.OLD_IRON_KING),

    "Memory of The Old Iron King": 
        RegionData(None, dlc=DLC.OLD_IRON_KING),

    "Shaded Woods": 
        RegionData([
            "Doors of Pharros",
            "Aldias Keep",
            "Drangleic Castle",
        ]),

    "Doors of Pharros": 
        RegionData(["Tseldora"]),

    "Tseldora": 
        RegionData(None),

    "Aldias Keep": 
        RegionData(["Dragon Aerie"]),

    "Dragon Aerie": 
        RegionData(["Dragon Shrine"]),

    "Dragon Shrine": 
        RegionData(None),

    "Drangleic Castle": 
        RegionData([
            "Shrine of Amana",
            "Eleum Loyce",
            "Throne of Want"
        ]),

    "Shrine of Amana": 
        RegionData(["Undead Crypt"]),

    "Undead Crypt": 
        RegionData(None),

    "Throne of Want": 
        RegionData(None),

    "Eleum Loyce": 
        RegionData(["Frigid Outskirts"], dlc=DLC.IVORY_KING),

    "Frigid Outskirts": 
        RegionData(None, dlc=DLC.IVORY_KING),
}

for region_name, region_data in region_connections.items():
    if not region_data.connections: continue
    for connection in region_data.connections:
        assert connection in region_connections.keys(), f"Invalid region: {connection} ({region_name} -> {connection})"