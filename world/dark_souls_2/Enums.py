from enum import Enum

class ItemCategory(Enum):
    ARMOR = 1
    CONSUMABLE = 2
    USABLE_ITEM = 3
    UPGRADE_MATERIAL = 4
    GESTURE = 5
    KEY_ITEM = 6
    WEAPON = 7
    AMMO = 8
    RING = 9
    SHIELD = 10
    SPELL = 11
    STAFF_OR_CHIME = 12
    STATUE = 13 
    HEALING = 14

class DLC(Enum):
    SUNKEN_KING = 1
    OLD_IRON_KING = 2
    IVORY_KING = 3
    ALL = 4