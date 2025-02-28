# Dark Souls II Archipelago

Dark Souls II client and world implementations for the [Archipelago multiworld randomizer](https://archipelago.gg/). Currently supports both the vanilla and scholar of the first sin version of the game.

## How to Use

### Installing the mod

The mod works with a single `dinput8.dll` file.

- Download the dll and apworld files from the [latest release](https://github.com/WildBunnie/DarkSoulsII-Archipelago/releases) for your game version.
- Rename the dll file to `dinput8.dll`.
- Place the `dinput8.dll` file inside the `Game` folder in the game folder, next to the executable.
- If playing on linux add `WINEDLLOVERRIDES="dinput8.dll=n,b" %command%` to the game's launch options on steam.

### Generating the world

- Download the lastest version of the archipelago client available [here](https://github.com/ArchipelagoMW/Archipelago/releases/latest).
- Inside the archipelago client, press `Install APWorld` and select the apworld you downloaded.
- Press `Generate Template Options` and grab the `Dark Souls II.yaml`.
- Edit the settings on the yaml file to your liking, especially changing your name and selecting the version of the game you will be playing.
- Press `Browse Files` in the client and place your yaml file and any others from people you might be playing with inside the `Players` folder.
- Press `generate` in the client to generate the world.
- Now either choose `host` to host the game locally or upload the file in the `output` folder to [archipelago's website](https://archipelago.gg/uploads).

### Joining a game

- (Optional) Backup your save just to make sure the mod doesn't mess with it.
- Simply launch the game and a console will launch together with it.
- In that console type `/connect server_address:port slot_name password`, replacing the correct values. The password is optional and the slot name is the name you placed in the yaml file.
- For example, if you host in archipelago's website it would look something like `/connect archipelago.gg:123456 JohnSouls`.
- Start a new game and enjoy.

## Building Locally

- clone the repository
- run `git submodule update --init --recursive` to download the submodules
- make sure you have vcpkg, it should be installed together with visual studio, and then run `vcpkg integrate install`
- set the correct platform (x86 for vanilla, x64 for scholar) and build

## Credits

https://github.com/SeanPesce/DLL_Wrapper_Generator \
https://github.com/black-sliver/apclientpp \
https://github.com/pseudostripy/DS2S-META

## Special Thank you

[pseudostripy](https://github.com/pseudostripy) (developer of [META](https://github.com/pseudostripy/DS2S-META)) for answering all my questions and helping out when i was lost
