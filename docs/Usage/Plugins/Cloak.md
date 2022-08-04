## Summary
The "Cloak" plugin allows players to cloak their ship if fitted with an appropriate cloaking device.

## Player Commands
All commands are prefixed with '/' unless explicitly specified.
- cloak - Cloaks/uncloaks the ship.

## Admin Commands
There are no admin commands in this plugin.

## Configuration
Default Configuration:
```json
{
    "cloakingDevices": {
        "example": {
            "cooldownTime": 0,
            "dropShieldsOnUncloak": false,
            "fuelToUsage": {
                "commodity_prisoners": 1
            },
            "holdSizeLimit": 0,
            "warmupTime": 0
        }
    },
    "dsAce": false
}
```

## IPC Interfaces Exposed
This plugin does not expose any functionality.

## Optional Plugin Dependencies
None

## Other
Documentation for the plugin code can be found [here](group___cloak.html).