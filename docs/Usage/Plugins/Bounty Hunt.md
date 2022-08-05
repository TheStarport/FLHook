## Summary
The "Bounty Hunt" plugin allows players to put bounties on each other that can be collected by destroying that player.

## Player Commands
All commands are prefixed with '/' unless explicitly specified.
- bountyhunt <player> <amount> [timelimit] - Places a bounty on the specified player. When another player kills them, they gain <credits>.
- bountyhuntid <id> <amount> [timelimit] - Same as above but with an id instead of a player name. Use /ids

## Admin Commands
There are no admin commands in this plugin.

## Configuration
Default Configuration:
```json
{
    "enableBountyHunt": true,
    "levelProtect": 0
}
```

## IPC Interfaces Exposed
This plugin does not expose any functionality.

## Optional Plugin Dependencies
None

## Other
Documentation for the plugin code can be found [here](@ref BountyHunt).