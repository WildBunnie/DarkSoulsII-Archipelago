from enum import Enum

from enum import Enum

class ItemCategory(Enum):
    MELEE_WEAPON = 1
    RANGED_WEAPON = 2
    MISC_WEAPON = 3
    STAFF = 4
    CHIME = 5
    SHIELD = 6
    HEAD_ARMOR = 7
    CHEST_ARMOR = 8
    HANDS_ARMOR = 9
    LEGS_ARMOR = 10
    RING = 11
    ARROW = 12
    GREAT_ARROW = 13
    BOLT = 14
    SPELL = 15
    UNIQUE = 16
    GOOD = 17
    FLASK_UPGRADE = 18
    BOSS_SOUL = 19
    GESTURE = 20
    STATUE = 21

class DLC(Enum):
    SUNKEN_KING = 1
    OLD_IRON_KING = 2
    IVORY_KING = 3
    ALL = 4