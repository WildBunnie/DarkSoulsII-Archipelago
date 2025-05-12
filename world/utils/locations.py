from collections import defaultdict
import json
import re

files = ["ItemLotParam2_Chr.csv","ItemLotParam2_Other.csv","ShopLineupParam.csv"]

meta_names = {}
regions = {}
with open("meta.csv", 'r', encoding='utf-16') as file:
    next(file)
    for line in file:
        location_id = int(line.split(",")[0])
        region = line.split(",")[1].strip()
        location_name = line.split(",")[2].strip()

        meta_names[location_id] = location_name
        regions[location_id] = region

item_names = {}
with open("sotfs/ItemParam.csv") as file:
    next(file)
    for line in file:
        item_names[int(line.split(",")[0])] = line.split(",")[1].strip()

ap_to_game = defaultdict(int)
game_to_ap = defaultdict(list)
locations = defaultdict(list)
    
region_names = {
    "ThingsBetwixt":"TB",
    "Majula":"MA",
    "FOFG":"FOFG",
    "Tseldora":"BCT",
    "AldiasKeep":"AK",
    "TheLostBastille":"LB",
    "HarvestValley":"HV",
    "EarthenPeak":"EP",
    "NoMansWharf":"NMW",
    "IronKeep":"IK",
    "HuntsmansCopse":"HC",
    "TheGutter":"GUT",
    "DragonAerie":"DA",
    "HeidesTowerOfFlame":"HTF",
    "ShadedWoods":"SW",
    "DoorsOfPharros":"DP",
    "ThePit":"PIT",
    "ShrineOfAmana":"SA",
    "DrangleicCastle":"DC",
    "UndeadCrypt":"UC",
    "BelfryLuna":"BL",
    "UndeadPurgatory":"UP",
    "GraveOfSaints":"GS",
    "MemoryOfJeigh":"MJ",
    "BlackGulch":"BG",
    "DragonShrine":"DS",
    "BelfrySol":"BS",
    "MemoryOfOrro":"MO",
    "MemoryOfVammar":"MV",
}

bundle_ids = set()
items_with_amounts = set()
current_id = 10000

def fix_shop_name(s):
    match = re.match(r'^\[(.*?)\]\s*(.*)$', s) or re.match(r'^(.*?)\s*\[(.*?)\]$', s)
    if not match:
        return s  # return unchanged if no brackets found

    if s.startswith('['):
        info, item = match.groups()
    else:
        item, info = match.groups()

    # Replace dashes with "in"/"after" and "and"
    parts = [part.strip() for part in info.split('-')]
    if len(parts) > 1:
        connector = "in" if parts[1] in region_names else "after"
        info_reworded = f"{parts[0]} {connector} {parts[1]}"
        if len(parts) > 2:
            info_reworded += ' and ' + ' and '.join(parts[2:])
    else:
        info_reworded = parts[0]

    return info_reworded

def create_location(location_id, location_name, item_id, chance, amount, sotfs=False, vanilla=False, keep_id=False):
    global current_id

    if location_id < 200000000 and chance != 100:
        return
    elif 200000000 <= location_id < 300000000 and (chance != 1 or "[Crows]" in location_name):
        return
    elif 300000000 <= location_id < 400000000 and item_id not in item_names:
        return
    
    if amount == 0:
        return

    item_name = item_names[item_id].replace("- ",", ")
    item_name += f" x{amount}" if amount != 1  else ""

    if amount > 1 and location_id < 300000000:
        bundle_ids.add(item_id+amount)
        items_with_amounts.add(f"ItemData({item_id + amount}, \"{item_name}\", ItemCategory.CONSUMABLE, amount={amount}),")

    region = "Undefined" if location_id not in regions else regions[location_id]
    
    if 300000000 <= location_id < 400000000:
        location_name = fix_shop_name(location_name)

    location_name = re.sub(r' \[REG:\d+\]', '', location_name)
    location_name = re.sub(r' \[GEN:\d+\]', '', location_name)
    location_name = location_name.replace(" (NPC)","")

    if location_id < 200000000:
        location_name += " (drop)"

    prefix = f"[{region}]" if region not in region_names else f"{region_names[region]}:"
    if location_id < 300000000:
        if location_name.startswith("["):
            location_name = re.sub(r'^\[.*?\]\s*', f'{prefix} {item_name} - ', location_name, count=1)
    else:
        location_name = f'{prefix} {item_name} - ' + location_name

    location = f"LocationData({current_id}, \"{location_name}\""
    location += ", missable=True" if location_id < 200000000 else ""
    location += ", ngp=True" if " ng+" in location.lower() else ""
    location += ", sotfs=True" if sotfs else ", vanilla=True" if vanilla else ""
    location += ")"

    # if location_id >= 300000000 and amount != 1:
    #     location = "#" + location

    locations[region].append(location)

    game_to_ap[location_id].append(current_id)

    if not keep_id:
        current_id += 1

offset = 0
for file in files:
    offset += 100000000 # offset needed cause ingame ids overlap between params
    with open(f'vanilla/{file}', 'r') as f1, open(f'sotfs/{file}', 'r') as f2:
        #skip csv headers
        next(f1)
        next(f2)

        line1 = f1.readline()
        line2 = f2.readline()

        while line1 or line2:

            vanilla = False
            sotfs = False
    
            data1 = line1.split(",")
            data2 = line2.split(",")
            id1 = int(data1[0])
            id2 = int(data2[0])

            # deal with locations that are only in 1 version
            if id1 != id2:
                if id1 < id2:
                    line1 = f1.readline()
                    location_id = int(data1[0])
                    vanilla = True
                else:
                    line2 = f2.readline()
                    location_id = int(data2[0])
                    sotfs = True
            else:
                line1 = f1.readline()
                line2 = f2.readline()
                location_id = int(data2[0])
            
            location_id = location_id + offset

            if offset != 300000000: # not shops

                location_name1 = data1[1] if location_id not in meta_names else meta_names[location_id]
                location_name2 = data2[1] if location_id not in meta_names else meta_names[location_id]
                
                for i in range(10):
                    if vanilla:
                        item_id = int(data1[46+i])
                        chance = float(data1[56+i])
                        amount = int(data1[6+i])
                        create_location(location_id, location_name1, item_id, chance, amount, vanilla=True)
                    elif sotfs:
                        item_id = int(data2[46+i])
                        chance = float(data2[56+i])
                        amount = int(data2[6+i])
                        create_location(location_id, location_name1, item_id, chance, amount, sotfs=True)
                    else:
                        item_id1 = int(data1[46+i])
                        item_id2 = int(data2[46+i])
                        chance1 = float(data1[56+i])
                        chance2 = float(data2[56+i])
                        amount1 = int(data1[6+i])
                        amount2 = int(data2[6+i])
                        if item_id1 != item_id2:
                            create_location(location_id, location_name1, item_id1, chance1, amount1, vanilla=True, keep_id=True)
                            create_location(location_id, location_name2, item_id2, chance2, amount2, sotfs=True)
                        else:
                            create_location(location_id, location_name2, item_id2, chance2, amount2)
            else:
                location_name1 = data1[1]
                location_name2 = data2[1]
                
                if vanilla:
                    item_id = int(data1[2])
                    amount = int(data1[-1])
                    create_location(location_id, location_name1, item_id, -1, amount, vanilla=True)
                elif sotfs:
                    item_id = int(data2[2])
                    amount = int(data2[-1])
                    create_location(location_id, location_name2, item_id, -1, amount, sotfs=True)
                else:
                    item_id1 = int(data1[2])
                    item_id2 = int(data2[2])
                    amount1 = int(data1[-1])
                    amount2 = int(data2[-1])
                    if item_id1 != item_id2:
                        create_location(location_id, location_name1, item_id1, -1, amount1, vanilla=True, keep_id=True)
                        create_location(location_id, location_name2, item_id2, -1, amount2, sotfs=True)
                    else:
                        create_location(location_id, location_name2, item_id2, -1, amount2)


for region in locations:
    print(f"    \"{region}\": [")
    for location in locations[region]:
        print(f"        {location},")
    print("    ],")

lines = [f"std::map<int, std::vector<int>> ap_location_map = {{"]
for key, values in game_to_ap.items():
    value_list = ", ".join(str(v) for v in values)
    lines.append(f"    {{{key}, {{{value_list}}}}},")
lines.append("};")
for line in lines:
    print(line)


s = sorted(bundle_ids)
print(set(s))

s = sorted(items_with_amounts, key=lambda x: x.split(",")[0])
for i in s:
    print(i)