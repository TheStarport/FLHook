Socket connections may be set to eventmode by entering `eventmode` as a command from one. From then on, the socket will receive several event notifications (listed below). Once activated,
event mode runs until you close the connection.

* `chat from=<player> id=<client-id> type=<type> [to=<recipient> idto=<recipient-client-id>] text=<text>`
  * `player`: Character name sending the message
  * `client-id`: Client ID of sender
  * `type`: Either `universe`, `system`, `group` or `player`
  * `recipient`/`recipient-client-id`: Only sent when `type` is `player`
  * `text`: Raw text of the message

* `kill victim=<player> type=<type> [by=<killer>]`
  * `player`: Character name of the victim
  * `type`: Either `selfkill`, `player`, `npc` or `suicide`
  * `killer`: Character name of the killer, if `type` is `player`

* `login char=<player> accountdirname=<dirname> id=<client-id> ip=<ip>`
  * Occurs when player selects a character in the character selection menu
  * `player`: Character name of the newly selected character
  * `dirname`: Short directory name of the player's account
  * `client-id`: Client ID of the player
  * `ip`: Public IP of the player

* `launch char=<player> id=<client-id> base=<basename> system=<systemname>`
  * Occurs when a player undocks from a base/planet
  * `player`: Character name of the launching player
  * `client-id`: Client ID of the player
  * `basename`: Name of the base being launched from
  * `systemname`: System name the character is now in

* `baseenter/baseexit char=<player> id=<client-id> base=<basename> system=<systemname>`
  * Occurs when player enters/exits a base (includes disconnect/F1)
  * `player`: Character name
  * `client-id`: Client ID of the player
  * `basename`: Name of the base
  * `systemname`: System name the base is in

* `jumpin/switchout char=<player> id=<client-id> system=<systemname>`
  * Occurs when player jumps in/out a system
  * `player`: Character name
  * `client-id`: Client ID of the player
  * `systemname`: Destination/origin system

* `spawn char=<player> id=<client-id> system=<systemname>`
  * Occurs when player selects a character and launches in space
  * `player`: Character name
  * `client-id`: Client ID of the player
  * `systemname`: System spawned into

* `connect id=<client-id> ip=<ip>`
  * Occurs when player connects to the server
  * `client-id`: Client ID of the player
  * `ip`: Public IP of the player

* `disconnect char=<player> id=<client-id>`
  * Occurs when player disconnects from the server
  * `client-id`: Client ID of the player
  * `player`: Character name at time of disconnect