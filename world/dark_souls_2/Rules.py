connection_rules = [
    # MAJULA
    {
        "from": "Majula",
        "to": "The Pit",
        "state": lambda state, player: (state.has("Silvercat Ring", player) or state.has("Flying Feline Boots", player))
    },
    {
        "from": "Majula",
        "to": "Huntsmans Copse",
        "state": lambda state, player: state.has("Rotunda Lockstone", player)
    },
    {
        "from": "Majula",
        "to": "Shaded Woods",
        "state": lambda state, player: state.has("Unpetrify Rosabeth of Melfia", player),
    },

    # FOREST OF FALLEN GIANTS
    {
        "from": "Forest of Fallen Giants",
        "to": "Forest of Fallen Giants - Soldier Key",
        "state": lambda state, player: state.has("Soldier Key", player),
    },
    {
        "from": "Forest of Fallen Giants",
        "to": "Forest of Fallen Giants - Salamander Pit",
        "state": lambda state, player: state.has("Iron Key", player),
    },
    {
        "from": "Forest of Fallen Giants - Soldier Key",
        "to": "Memory of Orro",
        "state": lambda state, player: state.has("Ashen Mist Heart", player),
    },
    {
        "from": "Forest of Fallen Giants - Soldier Key",
        "to": "Memory of Jeigh",
        "state": lambda state, player: all(state.has(item, player) for item in ["King's Ring", "Ashen Mist Heart"])
    },

    # SHADED WOODS
    {
        "from": "Shaded Woods",
        "to": "Aldia's Keep",
        "state": lambda state, player: state.has("King's Ring", player),
    },
    {
        "from": "Shaded Woods",
        "to": "Drangleic Castle",
        "state": lambda state, player: state.has("Open Shrine of Winter", player),
    },

    # DRANGLEIC CASTLE
    {
        "from": "Drangleic Castle",
        "to": "King's Passage",
        "state": lambda state, player: state.has("Key to King's Passage", player),
    },
    {
        "from": "Throne of Want",
        "to": "King's Passage",
        "state": lambda state, player: state.has("King's Ring", player),
    },

    # IRON KEEP
    {
        "from": "Iron Keep",
        "to": "Belfry Sol",
        "state": lambda state, player: state.has("Master Lockstone", player),
    },
    {
        "from": "Iron Keep",
        "to": "Brume Tower",
        "state": lambda state, player: state.has("Heavy Iron Key", player),
        "game_version": "sotfs" # TODO: FIX VANILLA STARTING WITH THE KEYS
    },
]