## Summary
The "Cargo Drop" plugin handles consequences to a player who disconnects whilst in space.

## Player Commands
There are no player commands in this plugin.

## Admin Commands
There are no admin commands in this plugin.

## Configuration
Default Configuration:
```json
{
    "cargoDropContainer": "lootcrate_ast_loot_metal",
    "disconnectMsg": "%player is attempting to engage cloaking device",
    "disconnectingPlayersRange": 5000.0,
    "hullDrop1NickName": "commodity_super_alloys",
    "hullDrop2NickName": "commodity_engine_components",
    "hullDropFactor": 0.1,
    "killDisconnectingPlayers": true,
    "lootDisconnectingPlayers": true,
    "noLootItems": [],
    "reportDisconnectingPlayers": true
}
```

## IPC Interfaces Exposed
This plugin does not expose any functionality.

## Optional Plugin Dependencies
None

## Other
Documentation for the plugin code can be found [here](@ref CargoDrop).