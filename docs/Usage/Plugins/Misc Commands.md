## Summary
The "Misc Commands" plugin provides an array of useful commands that serve as general quality of life features or game enhancements. They are commands that do not really fit into another plugin, or have the significance to belong in FLHook Core.

## Player Commands
All commands are prefixed with '/' unless explicitly specified.
\- lights - Activate optional ship lights, these usually default to ones on the docking light hardpoints.
\- shields - De/Reactivate your shields
\- pos - Prints the current absolute position of your ship
\- stuck - Nudges the ship 10m away from where they currently are while stationary. Designed to prevent player capital ships from being wedged.
\- droprep - Lowers the reputation of the current faction you are affiliated with. 
\- coin - Toss a coin and print the result in local chat.
\- dice [sides] - Tosses a dice with the specified number of sides, defaulting to 6.

## Admin Commands
\- smiteall [die] - Remove all shields of all players within 15k and plays music. If [die] is specified, then instead of lowering shields, it kills the players.

## Configuration
Default Configuration:
```json
{
    "coinMessage": "%player tosses %result",
    "diceMessage": "%player rolled %number of %max",
    "repDropCost": 0,
    "smiteMusicId": "music_danger",
    "stuckMessage": "Attention! Stand Clear. Towing %player"
}
```

## IPC Interfaces Exposed
This plugin does not expose any functionality.

## Other
Documentation for the plugin code can be found [here]($relpath^namespace_plugins_1_1_misc_commands.html).