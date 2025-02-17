from dataclasses import dataclass
from Options import PerGameCommonOptions, DeathLink
    
@dataclass
class DS2Options(PerGameCommonOptions):
    death_link: DeathLink