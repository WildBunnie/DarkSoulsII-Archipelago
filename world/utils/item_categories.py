from enum import Enum
from collections import defaultdict
import json

class GameItemCategory(Enum):
    WEAPON = 0
    SHIELD = 1
    HEAD_ARMOR = 2
    CHEST_ARMOR = 3
    HANDS_ARMOR = 4
    LEGS_ARMOR = 5
    AMMUNITION = 6
    RING = 7
    GOOD = 8
    SPELL = 9
    GESTURE = 10

class MyItemCategory(Enum):
    melee_weapons = 0
    shields = 1
    head_armor = 2
    chest_armor = 3
    hands_armor = 4
    legs_armor = 5
    bolts = 6
    rings = 7
    goods = 8
    sorceries = 9
    gestures = 10
    staffs = 11
    chimes = 12
    bows = 13
    crossbows = 14
    hexes = 15
    piromancies = 16
    miracles = 17
    misc_weapons = 18
    arrows = 19
    great_arrows = 20
    great_bows = 21
    boss_souls = 22
    usable_items = 23
    key_items = 24
    skip = 25

items_by_category = defaultdict(list)
items_to_ignore = [1831000, 3251000, 60155010, 60155020, 60155030] # duplicates and random estus flasks
usable_items = [53200000, 6100000, 60155000, 60355000, 60360000, 60405000, 60405010, 60406000, 60406010, 60420000, 60470000, 60480000, 60490000, 60500000, 62000000, 62020000, 62030000, 62040000, 62045000, 62070000, 62160000]
key_items = [50600000, 50610000, 50800000, 50810000, 50820000, 50830000, 50840000, 50850000, 50860000, 50870000, 50890000, 50900000, 50910000, 50930000, 50940000, 50950000, 50970000, 50990000, 51000000, 51030000, 52000000, 52100000, 52200000, 52300000, 52400000, 52500000, 52650000, 53100000, 53600000, 60536000, 62190000]
skip_items = [5600000, 5610000, 5620000, 5630000, 5640000, 5650000, 5660000, 40450000, 40710000, 40720000, 50940000, 60155000, 60420000, 60470000, 60480000, 60490000, 60500000, 60510000, 60537000, 62000000, 62020000, 62030000, 62040000, 62045000, 62050000, 62060000, 62070000, 62100000, 62110000, 62120000, 62130000, 62140000, 62150000, 62160000, 62170000]

with open("sotfs/ItemParam.csv") as file:
    next(file)
    for line in file:
        data = line.split(",")

        item_id = int(data[0])
        if item_id < 1000000 or item_id > 64610000: continue # unused items
        if item_id in items_to_ignore: continue

        item_name = data[1]
        if item_name.lower() in ["unknown",""]: continue

        category_id = int(data[26])
        if category_id not in GameItemCategory: continue
        game_category = GameItemCategory(category_id)

        # exceptions
        if item_id in usable_items:
            category = MyItemCategory.usable_items
        elif item_id in key_items:
            category = MyItemCategory.key_items
        elif item_id in skip_items:
            category = MyItemCategory.skip
        elif game_category == GameItemCategory.WEAPON:
            if item_id <= 3660000 or item_id == 3910000: # 3910000 is Pilgrim's Spontoon
                category = MyItemCategory.melee_weapons
            elif item_id <= 4150000 or item_id == 5540000:
                if "staff" in item_name.lower() or item_id == 3820000: 
                    category = MyItemCategory.staffs
                elif "chime" in item_name.lower() or item_id == 4020000:
                    category = MyItemCategory.chimes
                else:
                    print("idk what this item is", item_name, item_id)
            elif item_id <= 4290000:
                category = MyItemCategory.bows
            elif item_id <= 4440000:
                category = MyItemCategory.great_bows
            elif item_id <= 4680000:
                category = MyItemCategory.crossbows
            else:
                category = MyItemCategory.melee_weapons
        elif game_category == GameItemCategory.SPELL:
            if item_id <= 31310000:
                category = MyItemCategory.sorceries
            elif item_id <= 32310000:
                category = MyItemCategory.miracles
            elif item_id <= 33320000:
                category = MyItemCategory.piromancies
            elif item_id <= 35310000:
                category = MyItemCategory.hexes
        elif game_category == GameItemCategory.AMMUNITION:
            if item_id <= 60830000:
                category = MyItemCategory.arrows
            elif item_id <= 60900000:
                category = MyItemCategory.great_arrows
            elif item_id <= 60960000:
                category = MyItemCategory.bolts
        elif game_category == GameItemCategory.GOOD and (item_id == 53300000 or item_id >= 64000000):
            category = MyItemCategory.boss_souls
        else:
            category = MyItemCategory(category_id)

        if item_id == 6100000 or item_id == 5400000 or item_id == 5410000: category = MyItemCategory.misc_weapons
        items_by_category[str(category.name)].append(item_id)

for category in items_by_category:
    if category in ["gestures"]: continue
    sorted_items = sorted(items_by_category[category])
    items_string = str(sorted_items)
    print(f"const std::set<int32_t> {category} = {items_string.replace("[","{").replace("]","}")};")