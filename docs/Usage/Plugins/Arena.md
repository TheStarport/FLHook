## Summary
The "Arena" plugin provides commands to beam the player to a configurable base in a pvp system. This allows players to have an instant arena to do pvp even if they are far away from each other.

## Player Commands
All commands are prefixed with '/' unless explicitly specified.
- arena (configurable) - This beams the player to the pvp system.
- return - This returns the player to their last docked base.

## Admin Commands
There are no admin commands in this plugin.

## Configuration
Default Configuration:
```json
{
    "command": "arena",
    "restrictedSystem": "Li01",
    "targetBase": "Li02_01_Base",
    "targetSystem": "Li02"
}
```

## IPC Interfaces Exposed
This plugin does not expose any functionality.

## Optional Plugin Dependencies
This plugin uses the "Base" plugin.

## Other
Documentation for the plugin code can be found [here](group___arena.html).