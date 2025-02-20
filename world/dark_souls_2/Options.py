from dataclasses import dataclass
from Options import PerGameCommonOptions, DeathLink, Toggle
    
class EnableDLCsOption(Toggle):
    """Include items and locations exclusive to the DLCs"""
    display_name = "Enable DLCs"
    
class EnableNGPOption(Toggle):
    """Include items and locations exclusive to NewGame+ cycles"""
    display_name = "Enable NewGame+"

@dataclass
class DS2Options(PerGameCommonOptions):
    death_link: DeathLink
    enable_dlcs: EnableDLCsOption
    enable_ngp: EnableNGPOption