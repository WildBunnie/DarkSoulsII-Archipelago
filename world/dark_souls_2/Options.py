from dataclasses import dataclass
from Options import PerGameCommonOptions, DeathLink, Toggle, Choice
    
class EnableDLCsOption(Toggle):
    """Include items and locations exclusive to the DLCs"""
    display_name = "Enable DLCs"
    
class EnableNGPOption(Toggle):
    """Include items and locations exclusive to NewGame+ cycles"""
    display_name = "Enable NewGame+"

class GameVersion(Choice):
    """Choose the gave version you will be playing on
     
    - **sotfs:** You will be playing the scholar of the first sin version
    - **vanilla:** You will be playing the vanilla version
    """
    display_name = "Game Version"
    option_sotfs = 0
    option_vanilla = 1

@dataclass
class DS2Options(PerGameCommonOptions):
    death_link: DeathLink
    game_version: GameVersion
    enable_dlcs: EnableDLCsOption
    enable_ngp: EnableNGPOption