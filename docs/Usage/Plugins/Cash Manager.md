## Summary
The "Cash Manager" plugin allows players to send cash, receive cash and draw cash from other characters.

## Player Commands
All commands are prefixed with '/' unless explicitly specified.
- givecash <charname> <cash> [anon] - Sends credits to a player.
- sendcash <charname> <cash> [anon]- Alias of the above command.
- set cashcode <code> - Sets the "code" of this character. Other characters can withdraw cash from this character via /drawcash if they know this.
- showcash <charname> <code> - Show the cash currently on a character.
- drawcash <charname> <code> <cash> - Withdrawn cash from a character.

## Admin Commands
There are no admin commands in this plugin.

## Configuration
Default Configuration:
```json
{
    "blockedSystem": "Li01",
    "cheatDetection": false,
    "minimumTime": 0,
    "minimumTransfer": 0
}
```

## IPC Interfaces Exposed
This plugin does not expose any functionality.

## Optional Plugin Dependencies
None

## Other
Documentation for the plugin code can be found [here](group___cash_manager.html).