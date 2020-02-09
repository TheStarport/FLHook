================================================================================ 
Player Control plugin v1.0
================================================================================ 

This plugin merges all of the features from the givecash, renameme, repfixer,
and message plugins and adds purchase restrictions, testbot and player ship 
light/equipment changing.

Features:
- GUI based configuration
- Ability to change lights on player ships
- Restriction of ship purchase based on cargo item
- Restriction of good purchases based on base (to temporarily disable particular goods)
- Game banners
RepFixer:
- Automatically reset player reputations on launch or docking. The reputations are 
reset based on the presence of a specific equipment item.
Messaging:
- Preset messages to system, local system, group, current target chat.
- Message to last PM sender using fast reply.
- Message to current/last target
- Offline private message and private messaging by player name.
- Message to all players with faction tag
- Group invite all players with faction tag
- Automatic date-time display for each chat message.
- Optional custom help command to override FLHook's built in help
- Improved command usage reporting (including built in FL commands)
- /time and /setchattime command.
Cash:
- Allow a user to transfer cash to between characters using /givecash, /drawcash,
 /showcash, /setcashcode.
Rename commands:
- Allow a player to move a character from one account to another and to rename 
characters

The source for the plugin is located in the src folder.  You may use it for
whatever purpose you wish.  If you use it in one of your projects, I would 
appreciate being mentioned somewhere. 

You should be able to get the latest version of the FLHook plugin version from
http://forge.the-starport.net/projects/flhookplugin/files

This plugin is compatible with FLHook Plugin 1.6.0. I will release a 1.6.1 compatible
version when 1.6.1 comes out.

================================================================================
INSTALLATION
================================================================================

Copy the playercntl.dll into your flhook_plugins\dlls directory
Copy the playercntl.ini into your flhook_plugins\ directory
Copy the PlayerCntlSetup.exe into your flhook_plugins\ directory

Run the PlayerCntlSetup.exe and configure the plugin. When you click save, the
configuration will be written to the playercntl.ini file in the flhook_plugins
directory.

Type rehash in the flhook console to reload the configuration file.

================================================================================
USER-COMMANDS
================================================================================

/stuck
 Move a ship by approximately 10 meters.

/pos
 Prints the ship position to console

/droprep
 If you have an affiliated NPC faction, then this command drops your reputation 
 if that faction. You need to log out then in after using this command. Has no 
 effect on other factions.

/coin
 Flips a coin and reports the result to all ships within 6km

/dice [max]
 Shakes, rattles, rolls the dice and reports the result to all ships within 6km

/setmsg <N> <message>
  set message slot N to the specified message; if no message then the slot 
  is cleared and slots 1-9 are valid

/showmsgs
  show the message slot contents

/<N>
  send the message N to system chat

/l<N>
  send the message N to local system chat

/g<N>
  send the message N to group chat

/t<N>
  send the message N to the current/last target

/t <message> or /target
  send <message> to your last selected target

/r <message> or /reply
  send <message> to the last sender of your last private message

/pm <charname> <message> or /privatemsg
  send <message> to <charname>; if <charname> is offline the message will be
  delivered when they return

/pm$ <clientid> <message> or /privatemsg
  send <message> to <charname>; if <charname> is offline the message will be
  delivered when they return

/fm <tag> <message> or /factionmsg
  send message to all characters containing <tag> std::string in their names (like [RM]). 

/fi <tag> or /factioninvite 
  send an invite to all characters containing <tag> std::string in their names (like [RM]).

/set chattime <on|off>
  turn on in chat time stamps.

/time
  show current time in SMT (sirius mean time)

/mail
  read user's mail

/maildel
  delete user's mail.

/showrestarts
  A list of factions will be printed out. The factions are defined by .fl files
  in the restart directory. Note that .fl files starting with an '_' character 
  are not displayed but may still be used by the /restart command.

/restart <faction>
  Restart the current character using the specified faction.

/givecash <charname> <cash> [anon]
  give the cash to <charname>; if the 'anon' parameter is present then the receiving
  character will not be informed of who sent the transfer.
  
/setcashcode <code>
  set the account code to allow remote access to this characters account; if the
  code is 'none' then remote access is disabled.

/drawcash <charname> <code> <cash>
  draw the cash from <charname> with the <code> set using /setcashcode
  
/showcash <charname> <code>
  show the cash balance for <charname> with the <code> set using /setcashcode

/renameme <newcharname>
  rename current name to <newcharname> (player will be kicked)
 
/set movecharcode <charname> <code>
  sets the code to allow this character to be moved to another account.
  
/movechar <charname> <password>
  using the password set by the previous command, moves the specified character
  into this account.

/pimpship
 activate pimpship menu
 
If preset messages contain a #c then the #c will be replaced with the ship map 
coordinations.

If preset messages contain a #t then the #t will be replaced with current target
name.

================================================================================
ADMIN-COMMANDS
================================================================================

.smiteall
 Smites all ships within radar range.

.chase <charname>
 Moves an admin ship to the position of a specified ship. Sort of works across
 systems but it is recommended to beam to base in system and then use chase
 to jump to the player's location.

.pull <charname>
 Moves a player ship to the position of the ship issuing this command.

.testbot <system nickname>
 Starts a testing bot that jumps to all zones in the specified system. This is
 useful for finding patrol path and NPC encounter crashes.

.move <x> <y> <z>
 Move the admin ship to the specified coordinates 

.reloadbans
 Read the ip/id ban files.

.authchar <charname>
 Create a authenticated file in the account directory. Authenticated accounts
 skip the IP ban checking.

.setaccmovecode <charname> <code>
  sets the code for all characters in the account with this character in it

# NOT IMPLEMENTED
#.pm <charname> <message>
#  send <message> to <charname>; if <charname> is offline the message will be 
#  delivered when they return
   
================================================================================
LOGGING
================================================================================

Files ending with "-mail.ini" may be created in the user account 
directories. These files contain msgs for characters who are offline 
when a /pm command is issued.

Files ending with "-message.ini" may be created in the user account
directories. These files contain preset message setting for each character.

A file called "rename.ini" is created in the user accounts directory. This file
contains the time at which a character was renamed. This is used by the plugin
the enforce the time limit restriction but might be useful for an admin.

Reputation changes, ship movement, rename and cash transfers are logged in the
flhook.log file.

================================================================================
IP BANS
================================================================================

When a player is caught in an IP ban they will be able to connect to the 
server rather than not seeing it all of (which is what happens with peerguardian). 
When they login a message will be displayed to them saying "You are IP banned, 
contact an administrator." and then kicked after 15 seconds.

You can grant a known player access through a IP ban by either placing the file
"authenticated" in their account directory or typing "authchar <charname>" in the
flhook console window.

Edit the ipbans.ini to add or remove an IP ban. The path of this file is 
"\My Documents\My Games\Freelancer\Accts\MultiPlayer\ipbans.ini". After
editing and saving the file, type "reloadbans" in the flhook console window.

Lines in this file have the form
<ipaddress> = <optional comment>

The supported wild-characters in the '<ipaddress>' field are '*', '?'; sets: [a-z], '!' negation

The '= <optional comment>' does not have to be present

For example:
192.168.0.1 = Ban a single IP address
192.168.*.* = Ban a range

Edit the idbans.ini to add or remove an ID ban. The path of this file is 
"\My Documents\My Games\Freelancer\Accts\MultiPlayer\idbans.ini". After
editing and saving the file, type "reloadbans" in the flhook console window.

Lines in this file have the form
<id> = <optional comment>

See the flhook_kicks.log file for players who have been kicked because of an IP 
or ID ban.
 
================================================================================
OTHER FEATURES
================================================================================

- Logging the purchase of suspicious items.
- Logging the location of players during crashs.
- Prohibit purchase of goods from bases. A quick fix for broken commodity 
systems.
- Prints a message to all ships near  a ship that disconnects while in 
space. Optionally the plugin will eject the ship's cargo and/or destroy the ship.

================================================================================ 
CREDITS
================================================================================ 

- Mostly written by Cannon.
- Includes parts of the flak-common plugin by Motah
- Includes parts of FLHook by Wodk4 and various authors.
- The original /restart command by Jerzy
Thanks guys!

================================================================================ 
IMPORTING REPFIXER.INI SETTINGS
================================================================================ 
1. Copy the contents of repfixer.ini into playercntl.ini excluding the [Settings],
 [Hooks] and [General] sections.
2. Rename the section [FactionIDs] to [RepFixerItems]
3. Start the setup program and your settings should be there.

================================================================================ 
IMPORTING SHIP PURCHASE RESTRICTIONS
================================================================================ 
Make a spreadsheet with ship nicknames in the first row and good/equipment nicknames
in the first column.

If you put a 1 in the cell that intersects between a good/equipment nickname and 
a ship nickname, then the player must have the good/equipment item  to buy the ship.
Export the xls as csv and import the csv into the ship purchase screen. Delete 
any items in the ship purchase screen before you do the import.

================================================================================ 
CHANGE LOG
================================================================================ 

1.0:
...too much to mention, let's call this the first release.