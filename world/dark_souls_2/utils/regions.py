import re
items = {}
descriptions = {}

with open("vanilla/ItemParam.csv") as itemsFile:
    next(itemsFile) # skip csv header
    for line in itemsFile:
        data = line.split(",")
        name = re.sub(r'(?<!\s)- ', ', ', data[1])
        items[int(data[0])] = name

with open("meta.csv") as itemsFile:
    for line in itemsFile:
        data = line.split(',')

        # remove "vanilla" descriptions as they are
        # not of much utility to us
        description = data[1].strip()
        if "Vanilla" in description:
            description = re.sub(r'Vanilla.*: ',"", description)

            if "]" in description:
                split = description.split("] ")
                prefix = split[0] + "]"
                description = f"{prefix} {split[1].capitalize()}"
            else:
                description = description.capitalize()

        descriptions[int(data[0])] = description

regions = {}

with open("vanilla/ItemLotParam2_Other.csv") as itemLotFile:
    next(itemLotFile) # skip csv header

    checkedLocations = []
    for line in itemLotFile:
        data = line.split(",")
        itemLotId = int(data[0])
        locationName = data[1]
        description = descriptions[itemLotId]

        if itemLotId < 106000 or itemLotId > 60046001: continue
        # if itemLotId >= 3001000 and itemLotId <= 3021000: continue # skip gestures
        # if itemLotId >= 50000000 and itemLotId <= 50000303: continue # skip birds
        if locationName.startswith("[Unknown]"): continue
        if locationName.startswith("UNKNOWN"): continue

        prefix = locationName.split("] ")[0] + "]"
        if " - " in prefix:
            regionName = prefix.split(" - ")[0] + "]"
            regionName = regionName.replace("[","").replace("]","")
        else:
            regionName = prefix.replace("[","").replace("]","")
    
        if regionName not in regions:
            regions[regionName] = []

        if description not in checkedLocations:
            checkedLocations.append(description)
        else:
            amount = checkedLocations.count(description)
            checkedLocations.append(description)
            description = description + f" ({amount+1})"
    
        regions[regionName].append(f'LocationData({itemLotId}, "{description}")')


def writeToFile(file, text):
    file.write(f"{text}\n")

with open("locations.txt","w") as regionsFile:
    writeToFile(regionsFile, "location_table = {")
    for region in regions:
        writeToFile(regionsFile, f'    "{region}": [')
        for location in regions[region]:
            writeToFile(regionsFile, "        " + location + ",")
        writeToFile(regionsFile, "    ],")
    writeToFile(regionsFile, "}")