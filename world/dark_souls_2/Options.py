from dataclasses import dataclass
from Options import PerGameCommonOptions, DeathLink, Toggle
    
class EnableDLCsOption(Toggle):
    """Include items and locations exclusive to the DLCs"""
    display_name = "Enable DLCs"
    
@dataclass
class DS2Options(PerGameCommonOptions):
    death_link: DeathLink
    enable_dlcs: EnableDLCsOption