import re
items = {}
descriptions = {}

with open("sotfs/ItemParam.csv") as itemsFile:
    next(itemsFile) # skip csv header
    for line in itemsFile:
        data = line.split(",")
        items[int(data[0])] = data[1]

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

with open("sotfs/ItemLotParam2_Other.csv") as itemLotFile:
    next(itemLotFile) # skip csv header

    checkedLocations = {}
    for line in itemLotFile:
        data = line.split(",")
        itemLotId = int(data[0])
        locationName = data[1]
        description = descriptions[itemLotId]

        if itemLotId < 10025010 or itemLotId > 50376770: continue # only item pickups
        if itemLotId >= 50000000 and itemLotId <= 50000303: continue # skip birds
        if locationName.startswith("[Unknown]"): continue
    
        # # since its a csv all the ',' have been replaced with '-'
        # # we want them back but we need to make sure there is something before the '-'
        # # so that we dont pickup things in the prefix
        # locationName = re.sub(r'(?<!\s)- ', ', ', locationName)

        prefix = locationName.split("] ")[0] + "]"
        if " - " in prefix:
            regionName = prefix.split(" - ")[0] + "]"
            regionName = regionName.replace("[","").replace("]","")
        else:
            regionName = prefix.replace("[","").replace("]","")
    
        if regionName not in regions:
            regions[regionName] = []
            checkedLocations[regionName] = []

        if description not in checkedLocations[regionName]:
            checkedLocations[regionName].append(description)
        else:
            amount = checkedLocations[regionName].count(description)
            checkedLocations[regionName].append(description)
            description = description + f" ({amount+1})"

        multipleItems = False
        if int(data[47]) != 10: multipleItems = True

        for i in range(46, 56):
            itemId = int(data[i])
            if itemId in [0, 10]: continue # ignore empty
            itemName = items[itemId]

            finalItemLotId = itemLotId + i-46

            if multipleItems:
                count = i-45
                regions[regionName].append(f'({finalItemLotId}, "{description} (item {count})", "{itemName}")')
            else:
                regions[regionName].append(f'({finalItemLotId}, "{description}", "{itemName}")')


def writeToFile(file, text):
    file.write(f"{text}\n")

with open("regions.txt","w") as regionsFile:
    writeToFile(regionsFile, "location_table = {")
    for region in regions:
        writeToFile(regionsFile, f'    "{region}": [')
        for location in regions[region]:
            writeToFile(regionsFile, "        " + location + ",")
        writeToFile(regionsFile, "    ],")
    writeToFile(regionsFile, "}")