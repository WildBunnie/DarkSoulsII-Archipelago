from dataclasses import dataclass
from Options import PerGameCommonOptions, DeathLink, Toggle, Choice, ExcludeLocations, StartInventory, DefaultOnToggle

class NoWeaponRequirements(Toggle):
    """Remove the requirements to wield weapons"""
    display_name = "No Weapon Requirements"

class NoSpellRequirements(Toggle):
    """Remove the requirements to cast spells"""
    display_name = "No Spell Requirements"

class NoArmorRequirements(Toggle):
    """Remove the requirements to wear armor"""
    display_name = "No Armor Requirements"

class NoEquipLoad(Toggle):
    """Disable the equip load constraint from the game."""
    display_name = "No Equip Load"

class AutoEquip(Toggle):
    """Automatically equips any received armor, weapons or shields."""
    display_name = "Auto-Equip"

class RandomizeStartingLoadout(DefaultOnToggle):
    """Randomizes the equipment characters begin with."""
    display_name = "Randomize Starting Loadout"

class StartingWeaponRequirement(Choice):
    """Forces starting weapons to meet the chosen requirement."""
    display_name = "Starting Weapon Requirement"
    option_usable_with_one_hand = 0
    option_usable_with_two_hands = 1
    option_no_requirements = 2
    default = option_usable_with_two_hands


class OldIronKingDLC(Toggle):
    """Enable Crown of the Old Iron King DLC, randomizing items and locations within Brume Tower."""
    display_name = "Enable Crown of the Old Iron King DLC"

class IvoryKingDLC(Toggle):
    """Enable Crown of the Ivory King DLC, randomizing items and locations within Frozen Eleum Loyce."""
    display_name = "Enable Crown of the Ivory King DLC"

class SunkenKingDLC(Toggle):
    """Enable Crown of the Sunken King DLC, randomizing items and locations within Shulva."""
    display_name = "Enable Crown of the Sunken King DLC"
    
class EnableNGPOption(Toggle):
    """Include items and locations exclusive to NewGame+ cycles
    NOT RECOMMENDED, WORK IN PROGRESS
    """
    display_name = "Enable NewGame+"

class GameVersion(Choice):
    """Choose the game version you will be playing on
     
    - **sotfs:** You will be playing the scholar of the first sin version
    - **vanilla:** You will be playing the vanilla version
    """
    display_name = "Game Version"
    option_sotfs = 0
    option_vanilla = 1

class CombatLogic(Choice):
    """Determines the distribution of Estus Flask Shards and Sublime Bone Dust.
    Easy - Most shards/dust available fairly early on
    Medium - Moderate amount of shards/dust available early in the game
    Hard - There is minimal logical requirement for shards/dust to be available before the end of the game
    Disabled - There is zero requirements for shards/dust anywhere; Lost Bastille is Sphere 1 in Scholar, and Sinners' Rise is Sphere 1 in vanilla.
    """
    display_name = "Combat Logic"
    option_easy = 0
    option_medium = 1
    option_hard = 2
    option_disabled = 3
    default = option_medium

class EarlyBlacksmith(Choice):
    """Force Lenigrast's key into an early sphere in your world or across all worlds."""
    display_name = "Early Blacksmith"
    option_anywhere = 0
    option_early_global = 1
    option_early_local = 2
    default = option_early_local

class KeepInfiniteLifegems(Toggle):
    """Keep Melentia's infinite supply of lifegems"""
    display_name = "Keep Infinite Lifegems"

class DS2ExcludeLocations(ExcludeLocations):
    """Prevent these locations from having an important item."""
    default = frozenset({"Dark Chasm of Old"})

class DS2StartInventory(StartInventory):
    """Start with these items."""
    default = dict({"Torch":1})

@dataclass
class DS2Options(PerGameCommonOptions):
    game_version: GameVersion
    combat_logic: CombatLogic
    death_link: DeathLink
    no_weapon_req: NoWeaponRequirements
    no_spell_req: NoSpellRequirements
    no_armor_req: NoArmorRequirements
    no_equip_load: NoEquipLoad
    autoequip: AutoEquip
    randomize_starting_loadout: RandomizeStartingLoadout
    starting_weapon_requirement: StartingWeaponRequirement
    enable_ngp: EnableNGPOption
    early_blacksmith: EarlyBlacksmith
    infinite_lifegems: KeepInfiniteLifegems
    exclude_locations: DS2ExcludeLocations
    start_inventory: DS2StartInventory
    old_iron_king_dlc: OldIronKingDLC
    ivory_king_dlc: IvoryKingDLC
    sunken_king_dlc: SunkenKingDLC
