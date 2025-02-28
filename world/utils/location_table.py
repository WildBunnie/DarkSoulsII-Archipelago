import re

location_data_sotfs = {}
location_data_vanilla = {}

items_sotfs = {}
items_vanilla = {}

descriptions = {}
regions = {}

versions = ["sotfs","vanilla"]

with open("meta.csv") as itemsFile:
    for line in itemsFile:
        data = line.split(',')
        lot_code = int(data[0])
        description = data[1].strip()

        # the "vanilla" in the description refers to the default item for that location
        # which is mostly used for items important to speedruns but not of much utility to us
        if "Vanilla" in description:
            description = re.sub(r'Vanilla.*: ',"", description)

            if "]" in description:
                split = description.split("] ")
                prefix = split[0] + "]"
                description = f"{prefix} {split[1].capitalize()}"
            else:
                description = description.capitalize()

        descriptions[lot_code] = description

for version in versions:
    with open(f"{version}/ItemParam.csv") as items_file:
        next(items_file)
        for line in items_file:
            data = line.split(",")
            item_code = int(data[0])
            # the csv cant have ',' so they were replaced by '-'
            # so we change it back but only if it is '- '
            item_name = re.sub(r'(?<!\s)- ', ', ', data[1])

            items_dict = globals()[f"items_{version}"]
            items_dict[item_code] = item_name

    with open(f"{version}/ItemLotParam2_other.csv") as lots_file:
        next(lots_file)
        # keep track of how many times a description has been used
        # we use this to make sure we dont have duplicates
        description_amounts = {}
        for line in lots_file:
            data = line.split(",")

            lot_code = int(data[0])
            if lot_code < 106000 or lot_code > 60046001: continue
            if lot_code in [0, 10]: continue

            lot_name = data[1]
            if lot_name.lower() in ["unknown"]: continue

            description = descriptions[lot_code]
            if "unknown" in description.lower(): continue
            if description not in description_amounts:
                description_amounts[description] = 1
            else:
                description_amounts[description] += 1

            # get the region from the name
            prefix = lot_name.split("] ")[0] + "]"
            if " - " in prefix:
                region_name = prefix.split(" - ")[0] + "]"
                region_name = region_name.replace("[","").replace("]","")
            else:
                region_name = prefix.replace("[","").replace("]","")
        
            if region_name.lower() in ["unknown"]: continue

            if region_name not in regions:
                regions[region_name] = {lot_code}
            else:
                regions[region_name].add(lot_code)

            rewards = []
            items_dict = globals()[f"items_{version}"]
            # loop through rewards
            for i in range(46, 56):
                item_code = int(data[i])
                if item_code in [0, 10]: continue
                item_name = items_dict[item_code]
                rewards.append(item_name)
            
            if description_amounts[description] > 1:
                description = description + f" ({description_amounts[description]})"

            result = f'LocationData({lot_code}, "{description}", {rewards}'
            if " ng+" in description.lower():
                result += ", ngp=True"

            location_dict = globals()[f"location_data_{version}"]
            location_dict[lot_code] = result

print("location_table = {")
for region_name in regions:
    locations = regions[region_name]
    locations = sorted(locations)
    print(f'    "{region_name}": [')
    for location_code in locations:
        sotfs_data = location_data_sotfs[location_code]
        if location_code in location_data_vanilla:
            vanilla_data = location_data_vanilla[location_code]
        else:
            print("        " + sotfs_data + ", sotfs=True),")
            continue

        if sotfs_data == vanilla_data:
            print("        " + sotfs_data + "),")
        else:
            print("        " + vanilla_data + ", vanilla=True),")
            print("        " + sotfs_data + ", sotfs=True),")
    print('    ],')
print('}')

