# DarkSoulsII Archipelago Client and World

Dark Souls II client and world made for Archipelago multiworld randomizer. See [archipelago.gg](https://archipelago.gg/). This is currently only available for **VANILLA DARK SOULS II**, not the scholar of the first sin version.

# How to Use

## Installing the mod

The mod works with a single `dinput8.dll` file.

- Download the dll and apworld files from the [latest release](https://github.com/WildBunnie/DarkSoulsII-Archipelago/releases).
- Place the `dinput8.dll` file in your game folder, next to the executable.

## Generating the world

- Download the lastest version of the archipelago client available [here](https://github.com/ArchipelagoMW/Archipelago/releases/latest).
- Inside the archipelago client, press the `Install APWorld` and select the download apworld.
- Press the `Generate Template Options` and grab the `Dark Souls II.yaml`.
- Edit the settings on the yaml file to your liking, especially changing your name.
- Press the `Browse Files` option in the client and place your yaml file and any others from people you might be playing with inside the `Players` folder.
- Press `generate` in the client to generate the world.
- Now either choose `host` to host the game locally or upload the file in the `output` folder to [archipelago's website](https://archipelago.gg/uploads).

## Joining a game

- Simply launch the game and a console will launch together with it.
- In that console type `/connect server_address:port slot_name password`, replacing the correct values with the password being optional and the slot name being the name you placed in the yaml file.
- For example, if you host in archipelago's website it would look something like `/connect archipelago.gg:123456 JohnSouls`.

# Building Locally

- clone the repository
- run `git submodule update --init --recursive` to download the submodules
- make sure you have vcpkg, it should be installed together with visual studio, and then run `vcpkg integrate install`
- set the platform to x86 and build