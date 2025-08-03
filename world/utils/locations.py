import csv
from collections import defaultdict

def parse_csv_to_location_table(csv_path):
    location_table = defaultdict(list)
    name_counts = defaultdict(int)
    name_instances = defaultdict(list)

    with open(csv_path, newline='', encoding='utf-8') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            if row["done"] == "FALSE" or row["dont add"] == "TRUE":
                continue

            region = row["logical region"]
            loc_id = int(row["id"])
            description = row["final name"]
            missable = row["missable"].strip().upper() == "TRUE"

            # Track all instances of the same name
            name_counts[description] += 1
            name_instances[description].append((region, loc_id))

            # Build LocationData as a tuple
            data = [loc_id, description]
            if 200000000 <= loc_id < 300000000 and missable:
                data.append("missable=True")
            if row["version"] == "sotfs":
                data.append("sotfs=True")
            elif row["version"] == "vanilla":
                data.append("vanilla=True")

            location_table[region].append(data)

    # Rename duplicates by appending #1, #2, ...
    for name, count in name_counts.items():
        if count > 1:
            for i, (region, loc_id) in enumerate(name_instances[name], 1):
                for loc in location_table[region]:
                    if loc[0] == loc_id and loc[1] == name:
                        loc[1] = f"{name} #{i}"
                        break

    for region in location_table:
        location_table[region].sort(key=lambda x: x[0])

    return location_table

def print_location_table(location_table):
    print("location_table: Dict[str, List[LocationData]] = {")

    # First print regions in the specified order
    for region in location_table:
        if region in location_table:
            print(f'    "{region}": [')
            for loc in location_table[region]:
                base = f"        LocationData({loc[0]}, {repr(loc[1])}"
                extras = ", " + ", ".join(loc[2:]) if len(loc) > 2 else ""
                print(f"{base}{extras}),")
            print("    ],")

    print("}")

if __name__ == "__main__":
    table = parse_csv_to_location_table("locations.csv")
    print_location_table(table)
