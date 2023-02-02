Mining Control Plugin for FLHook-Plugin 1.6.0
by Cannon
=============
version: 1.1

A plugin for FLHook-plugin that allows a server based control over mining 
operations.

This plugin overrides the anticheat plugin's lootspawn check. This plugin 
provides a similar level of protection.

Credit to wodk4, M0tah, mc_horst and all of the hook guys. I've borrowed
bits of the software in this plugin from the flak plugin and flhook source.

The source for the plugin is located in the src folder.  You may use it for
whatever purpose you wish.  If you use it in one of your projects, I would
appreciate being mentioned somewhere.

You should be able to get the latest version of the FLHook plugin from
http://forge.the-starport.net/gf/project/flhook/

================================================================================
== INSTALLATION ================================================================
================================================================================
Copy the file bin\minecontrol.dll into the ...\freelancer\exe\flhook_plugins\dll directory
COpy the file bin\minecontrol.ini into the ...\freelancer\exe\flhook_plugins directory.

================================================================================
== CONFIG ======================================================================
================================================================================
See the minecontrol.ini file 

================================================================================
== ADMIN-COMMANDS ==============================================================
printminezones
 - shows information related to all default mineable zones

 ================================================================================
== SOURCE CODE =================================================================
================================================================================
- compiled with VS2003

================================================================================
== CHANGELOG ===================================================================
================================================================================

0.1
 First version 
0.2
 Use own asteriod field calculations.
0.3
 On loot-cheat make player's ship explode and log to flhook cheaters.log
0.4
 Fixed in-zone calculation.
 Added field by field bonus definitions
0.5:
 Fixed the fix for zone calculation problems
 Added commodity modification for fields
1.0:
 Gave up on my own zone calculations and went back to using the FL ones.
 Changed the bonuses to only work if all equipment items are present.
 Changed the configuration file format to make setup a little quicker.
 Removed autoban and replaced with autokill on cheat detection.
1.1:
 Fixed initialisation problems. Note that the plugin ini has changed parameters;
 compare your file to this.
 Remove type and count cheat detection as the plugin stops these cheats anyway
 and false detections were a problem.
 Added checking nicknames in player bonus section.
