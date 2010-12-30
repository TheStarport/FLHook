F L   H O O K
=============
version: 1.6.1 plugin

================================================================================
== INSTALLATION ================================================================
================================================================================
FLHOOK ONLY WORKS WITH FLSERVER 1.1. USING IT WITH 1.0 WILL CRASH FLSERVER!!!

copy the files from "bin" to your freelancer/exe directory and edit the FLHook.ini
in order to suite your needs.
then open dacomsrv.ini in "...\freelancer\exe" and append FLHook.dll to the 
   [Libraries] section. flserver will load flhook whenever you start it.
   
================================================================================
== PLUGIN INSTALLATION =========================================================
================================================================================
in a typical case plugins are distributed in an archive with a folder structure
that you just need to copy to the "Freelancer/EXE/" folder.
every plugin needs a configuration file within the "./EXE/flhook_plugins/" main folder,
the plugin DLL needs to be in ".EXE/flhook_plugins/dlls/".
please read any readme file that may be delivered with the plugin you want to
install!

================================================================================
== CONFIG ======================================================================
================================================================================
take a look at the comments in FLHook.ini

================================================================================
== ADMIN-COMMANDS ==============================================================
================================================================================
commands can be executed by administrators in several ways:
- using the FLHook console(-> access to all commands)
- ingame by typing .command in the chat(e.g. .getcash Player1)
  this will only work when you own the appropriate rights which may be set via
  the setadmin command. FLHook will store the rights of each player in his
  account-directory in flhookadmin.ini.
- via a socket connection in raw text mode(e.g. with putty)
  connect to the port given in the FLHook.ini and enter "PASS password". after
  having successfuly logged in you may enter all of the commands you want(as
  long as you have the neccessary rights). you may have several socket connections
  at the same time. exiting the connection may be done by entering "quit" or simply
  by closing it.

- CASH -
all cash functions work no matter if the player is currently logged in or not
getcash <charname>
  shows current account balance of <charname>
setcash <charname> <amount>
  sets current account balance of <charname> to <amount>
setcashsec <charname> <oldmoney> <amount>
  sets current account balance of <charname> to <amount>, only works when his old account balance is <oldmoney>
addcash <charname> <amount>
  adds <amount> to the current account balance of <charname>
addcashsec <charname> <oldmoney> <amount>
  adds <amount> to the current account balance of <charname>, only works when his old account balance is <oldmoney>

- KICK/BAN -
kick <charname> <reason>
  disconnects <charname>. the user will be displayed <reason>, if it is specified.
ban <charname>
  bans <charname>'s account
  <charname> stays connected if he's currently on the server
unban <charname>
  unbans <charname>'s account
kickban <charname> <reason>
  kicks and bans <charname> (2 in 1, same as kick <charname> <reason>, ban <charname>)

- MSG -
msg <charname> <text>
  private message <text> to <charname> (shown as "Console: <text>")
msgs <systemname> <text>
  send <text> to all players in <systemname> (shown as "Console: <text>")
  <systemname> must be the either the system-id or the shortname (like Li01)
msgu <text>
  message <text> to the whole universe (shown as "Console: <text>")
fmsg <charname> <xmltext>
  private message <xmltext> to <charname> (see XMLTEXT section for further details)
fmsgs <systemname> <xmltext>
  send <xmltext> to all players in <systemname> (see XMLTEXT section for further details)
fmsgu <xmltext>
  message <xmltext> to the whole universe (see XMLTEXT section for further details)

- BEAM/KILL -
beam <charname> <basename>
  force <charname> to land on <basename> (player must be in space)
  <basename> must be either the shortname(like Li01_01_Base for manhatten) or a shortcut defined in the FLHook.ini
kill <charname>
  kills <charname>

- REPUTATION -
resetrep <charname>
  sets <charname>'s reputations to the one specified in "mpnewcharacter.fl"
setrep <charname> <repgroup> <value>
  set <charname>'s reputation for <repgroup> to <value>. <value> should be between -1 and 1.
  example:
  "setrep playerxy li_n_grp 0.7"
  -> set playerxy's reputation for liberty navy to 0.7
setrep <charname> <repgroup>
  return the feelings of <repgroup> towards <charname>

- CARGO -
cargo commands only work when the targeted player is ingame
enumcargo <charname>
  lists <charname>'s cargo, first reply will be the remaining hold size
addcargo <charname> <good> <count> <mission>
  adds <count> numbers of <good>(shortname like co_gun01_mark02,commodity_silver,etc OR hash) to <charname>'s cargo
  if <mission> is set to 1, the cargo is declared as mission cargo
  note: see issues
removecargo <charname> <id> <count>
  removes <count> numbers of <id>(this must be the value from enumcargo's "id=" reply) from <charname>

- CHARACTERS -
rename <oldcharname> <newcharname>
  rename <oldcharname> to <newcharname> (player will be kicked if he's logged in <oldcharname>'s account)
deletechar <charname>
  delete <charname> (player will be kicked if he's logged in <oldcharname>'s account)
readcharfile <charname>
  reads <charname>'s userfile(xxx.fl) and prints it(each line will be preceded by "l ")
writecharfile <charname> <data>
  writes <data> into <charname>'s userfile(xxx.fl). existing charfile will be overwritten. you should
  be careful with this one because a corrupted charfile may lead to server crashes and flhook does 
  not do any syntax checks on <data>.
  NOTE: YOU MUST REPLACE LINE-WRAPS WITH \n(TEXT)
  example:
  writecharfile playerxy [Player]\ndescription = 00300034002f0031003\n\n ... etc.

- SETTINGS -
setadmin <charname> <rights>
  set <charname> as ingame-admin with <rights> (affects all characters on the account) (see RIGHTS section below)
getadmin <charname>
  show <charname>'s rights
deladmin <charname>
  revoke <charname>'s ingame-admin-status
getreservedslot <charname>
  show if <charname> has a reserved slot
setreservedslot <charname> <value>
  set reserved slot on <charname> (1=on, 0=off)
rehash
  reload the flhook.ini in order to activate changed settings, this works for everything except the socket-settings
  
- PLUGINS -
loadplugins
	loads all plugins in the plugin folder, reloads any unloaded plugins
listplugins
	lists all loaded plugins
unloadplugin <plugin shortname>
	unloads a certain plugin (if allowed)
pauseplugin <plugin shortname>
	pauses a certain plugin (if allowed)
unpauseplugin <plugin shortname>
	continues a paused plugin

- OTHER -
getgroupmembers <charname>
  returns all players which are in a group with <charname>
getbasestatus <basename>
  returns the hull status of a base. when the base hasn't been created in space yet it returns 0.
getclientid <charname>
  gets <charname>'s client id
getplayerinfo <charname>
  get <charname>'s info
xgetplayerinfo <charname>
  same as getplayerinfo, except that result is shown in a more readable format
getplayers
  get player info for all players on the server (players in charselect menu will not be shown)
xgetplayers
  same as getplayers, except that result is shown in a more readable format
getplayerids
  shows all players on the server with their client-id in a short format(useful when ingame)
getaccountdirname <charname>
  get account-dirname of <charname>
getcharfilename <charname>
  get char-filename of <charname>
savechar <charname>
  save <charname>'s current info to disk
isloggedin <charname>
  check if <charname> is logged in on the server
isonserver <charname>
  check if <charname> is connected(this includes idleness in charselect menu) to the server
  NOTE: isonserver will also return true, when another char on the same account is logged in!
serverinfo
  shows server load, whether npc spawn is currently enabled or disabled(see ini) and uptime.
  the format for the uptime is: days:hours:minutes:seconds
moneyfixlist
  show players with active money-fix
help
  get a list of all commands
  
- MISC INFOS -
you can use client-ids instead of <charname> by appending $ to the cmd.
examples: 
getcash$ 12
kickban$ 1
etc.

you can also use "shortcuts" instead of the whole character-name for a currently logged in player by appending
& to the cmd. FLHook traverses all logged in players and checks if their character-name contains
<charname>(case insensitive) and if so, the command will operate on this player. an error will be shown if the 
searchstring given in <charname> is ambiguous.
examples (let's assume there are 2 players logged in: "superhax0r" and "..::[]SUPERNERD[]::.."):
"kick& nerd" kicks "..::[]SUPERNERD[]::.." because his nick contains "nerd"
"getcash& super" fails because there are multiple character names containing "super"

all commands return "OK" when successful or "ERR <errortext>" when error occured

================================================================================
== RIGHTS ======================================================================
================================================================================
rights may be seperated by a comma (e.g. setadmin playerxy cash,kickban,msg)
superadmin        -> everything
cash              -> cash commands
kickban           -> kick/ban commands
beamkill          -> beam/kill/resetrep/setrep command
msg               -> msg commands
cargo             -> cargo commands
characters        -> character editing/reading commands
reputation		  -> reputation commands
eventmode         -> eventmode (only when connected via socket)
settings		  -> rehash and reserved slots (setadmin only with superadmin rights)
plugins			  -> plugin commands
other	          -> all other commands
special1		  -> special commands #1
special2		  -> special commands #2
special3		  -> special commands #3
all other commands except setadmin/getadmin/deladmin may be executed by all admins

================================================================================
== XMLTEXT =====================================================================
================================================================================

the fmsg* commands allow you to format text in several ways(like in the exe\misctext.dll)
text is enclosed in <TEXT></TEXT> tags while the format can be changed with <TRA .../>
nodes-names must be written in capital chars! 
be sure to replace the following characters within a text-node:
< -> &#60; 
> -> &#62;
& -> &#38;

- <TRA .../> NODE SYNTAX -
the data field of a TRA node consists of an RGB value along with format specifications:
<TRA data="0xBBGGRRFF" mask="-1"/>
BB is the blue value
GG is the green value
RR is the red value
FF is the format value
(all in hexadecimal representation)

format flags are:
bin      hex  dec  effect
00000001 1    1    bold
00000010 2    2    italic
00000100 4    4    underline
00001000 8    8    big
00010000 10   16   big&wide
00100000 20   32   very big
01000000 40   64   smoothest?
10000000 80   128  smoother?
10010000 90   144  small
simply add the flags to combine them (e.g. 7 = bold/italic/underline)

examples:
fmsgu <TRA data="0x1919BD01" mask="-1"/><TEXT>A player has died: Player</TEXT>
this is similar to the standard die-msg(which is shown in bold)
fmsgu <TRA data="0xFF000003" mask="-1"/><TEXT>Hello</TEXT><TRA data="0x00FF0008" mask="-1"/> <TEXT>World</TEXT>
this will show "Hello World" ("Hello" will be blue/bold/italic and "World" green/big)

================================================================================
== EVENTMODE ===================================================================
================================================================================
socket connections may be set to eventmode by entering "eventmode". from then on 
you will receive several event-notifications listed below. once activated, 
eventmode runs until you close the connection.

- NOTIFICATIONS -
chat from=<player> id=<client-id> type=<type> [to=<recipient> idto=<recipient-client-id>] text=<text>
  <player>: charname sending the message
  <client-id>: client-id of sender
  <type>: either universe,system or player
  <recipient>/<recipient-client-id>: only sent when type=player
  <text>: guess ...

kill victim=<player> type=<type> [by=<killer>]
  <player>: charname of the victim
  <type>: selfkill,player,npc,suicide
  <killer>: charname of the killer

login char=<player> accountdirname=<dirname> id=<client-id> ip=<ip>
  occurs when player selects a character in the character-select menu

launch char=<player> id=<client-id> base=<basename> system=<systemname>
  occurs when player undocks from a base/planet

baseenter char=<player> id=<client-id> base=<basename> system=<systemname>
  occurs when player enters base
  
baseexit char=<player> id=<client-id> base=<basename> system=<systemname>
  occurs when player exits base(includes disconnect/f1)

jumpin char=<player> id=<client-id> system=<systemname>
  occurs when player jumps in a system
  
switchout char=<player> id=<client-id> system=<systemname>
  occurs when player switches out a system

spawn char=<player> id=<client-id> system=<systemname>
  occurs when player selects a character and launches in space

connect id=<client-id> ip=<ip>
  occurs when player connects to the server

disconnect char=<player> id=<client-id>
  occurs when player disonnects from the server

================================================================================
== USER-COMMANDS ===============================================================
================================================================================
user commands may be entered ingame by every player in chat and can be enabled
or disabled in the ini. enter them ingame to get a description.

/set diemsg xxx
  while xxx must be one of the following values:
  - all = all deaths will be displayed
  - system = only display deaths occuring in the system you are currently in
  - self = only display deaths the player is involved in(either as victim or killer)
  - none = don't show any death-messages
  settings keep saved in flhookuser.ini and affect all characters on the account

/set diemsgsize <small/default>
  - change the size of the diemsgs

/set chatfont <size> <style>
  <size>: small, default or big
  <style>: default, bold, italic or underline
  this let's every user adjust the appearance of the chat-messages

/autobuy <...>
  let ammo be bought automatically whenever you enter a base. enter the command
  ingame to get a description

/ignore <charname> [<flags>]
  ignore chat from certain players

/ignoreid <client-id> [<flags>]
  ignore by client-id
  
/delignore <id> [<id2> <id3> ...]
  delete ignore entry
  
/ignorelist
  display ignore list
  
/ids
  show client-ids of all players

/id
  show own client-id
  
/i$ <client-id> and /invite$ <client-id>
  invite player to group by client-id
  
/credits
  show the authors of FLHook and all running plugins

/help
  show all available user commands

================================================================================
== LOGGING =====================================================================
================================================================================
flhook logfiles are created in "Freelancer/EXE/flhook_logs/".

flhook.log
  debug
flhook_kicks.log:
  idle-/ping-/loss-/corruptedcharfile-/ipban-/hostban-kicks
flhook_cheaters.log:
  detected cheating
flhook_connects.log:
  every login if activated in FLHook.ini
flhook_usercmds.log:
  every user command if activated in FLHook.ini
flhook_admincmds.log:
  every admin command if activated in FLHook.ini
flhook_perftimers.log:
  performance timer reports if activated in FLHook.ini
  
with a debug=yes setting in FLHook.ini, timestamped debug logfiles will be created
in "flhook_logs/debug/"

================================================================================
== KNOWN ISSUES ================================================================
================================================================================
- the sender in the standard chat messages appear a little bit smaller when
  "/set chatfont" is enabled

================================================================================
== SOURCE CODE =================================================================
================================================================================
FLHook compiles on both vc7 and vc6
Note:
flserver uses string-class arguments in some of its functions(f. e. 
PlayerDB::FindAccountFromCharacterName(...)). when you compile since FLHook with 
vc7, which uses a different stl, you can't simply pass strings. that's why
i created the FLHookWString.dll which was compiled with vc6 and just exports
two functions to create/delete flserver compatible strings. if you don't have
vc6 installed then use the FLHookWString.dll from bin and thats it.

if you want the full header files/libs of all of the relevant flserver dlls then 
take a look at FLCoreSDK(www.skif.be). it contains everything you need.

please note:
I WILL NOT TEACH YOU HOW TO CODE, I WILL NOT TELL YOU HOW TO REVERSE FLSERVER 
AND I WILL NOT REPLY TO "STUPID" QUESTIONS REGARDING THE SOURCE

================================================================================
== BLOWFISH ENCRYPTION PROGRAMMING =============================================
================================================================================
Because Blowfish only supports encrypting 8 bytes of data at once, it is
necessary to pad data (with 0x00s) that is not a multiple of 8 bytes.  When
recieving encrypted data from FLHook, you should check for null characters and
remove them from the decrypted string.  When sending encrypted data to FLHook,
you should pad the string with null characters if necessary.  If the blowfish
implementation you are using specifically encrypts and decrypts strings, this
may already be handled by it.

================================================================================
== TROUBLESHOOTING AND SUPPORT =================================================
================================================================================
for any help regarding FLHook or its plugins, please refer to
http://www.the-startport.net
and visit the FLHook development boards.

================================================================================
== CREDITS =====================================================================
================================================================================
programmed & documented by mc_horst
after version 1.5.5 programmed & documented by w0dk4
after version 1.6.0 development based on open-source SVN, supervised by w0dk4

BIG THX to w0dk4 and Niwo for testing and the whole Hamburg City Server adminteam 
for the suggestions and feedback

contact: find me on www.freelancerserver.de or #hc-flserver (quakenet)

FLHook uses a slighty modified version of flcodec.c

================================================================================
== CHANGELOG ===================================================================
================================================================================

1.6.1 plugin
=====
- fixed critical bug in built-in anticheat detection (w0dk4)
- fixed autobuy bug (w0dk4)
- fixed remaining bug with the rename command (Cannon)
- IFSO charfile compatability added (Cannon)
- added blowfish encryption option for the socket connection (M0tah)
- optimized Spawnprotection code (Schmackbolzen)
- added new plugin callbacks [_AFTER] (w0dk4)
- fixed user-defined format-strings vulnerabilities (OutCast)
- fixed unicode socket bug (Cannon)
- added exception catch block to pub::spaceobj::dock calls to suppress crashes (Cannon)
- added VC 2008 Solution, switching code & compilation to VC9 (w0dk4)
- added complete callback hooks for outgoing network packets (w0dk4)
- added exports for flcodec functions and GetShipInspect (M0tah)
- fixed crash when [Bans] section is missing in FLHook.ini (M0tah)
- fixed HkAddCash failing when wscCharname didn't match the character name's case exactly,
	the player was logged into that account with a different character at the time of the call,
	and the player later switched to that character without logging out. (M0tah)
- fixed /invite$ not working on characters with '<', '>', or '&' in their names (M0tah)
- fixed the new BaseDestroy socket event implementation - was throwing the event even when a base wasn't destroyed (M0tah)
- fixed memory leak crash bug in flserver (w0dk4)
- fixed missile jitter bug on modern cpus (w0dk4)
- reworked include/import header files based on FLCoreSDK by Skif (w0dk4)

1.6.0 plugin
=====
- fixed rename function
- fixed addcargo admin command (was causing cheat detect kick under certain circumstances)
- fixed autobuy now buying up to MAX_PLAYER_AMMO constant units of ammo
- death messages: killer should now be identified better on ships which use death fuses
- added some requested and additional exports for plugins
- reimplemented and fixed basedestroy (socket) event
- added "special" admin rights for special commands that may be implemented by plugins (see "rights" section)
- improved condata plugin
- implemented help command (with plugin callback)
- improved general logging (see log section and FLHook.ini)

1.5.9 plugin
=====
- fixed crash bug in beam command introduced with last version *sigh
- fixed a potential crash bug when shutting down FLServer
- included MSVC 7.0 runtime library files needed for FLHook
- Connection Data plugin: fixed lag detection
- updated FLHook.ini and added explanations of connection kick settings

1.5.8 plugin
=====
- fixed bugs in plugin loading function
- added new hooks (ElapseTime, UpdateGlobalTiming, LaunchPosHook, DockCall; see plugin documentation)
- reserved slot admin commands added to readme and help command
- log: fixed some logfiles not logging to "Freelancer/EXE/flhook_logs/"
- log: added optional connect logging (see FLHook.ini)
- code: HkEnumCargo now reports more information (see FLHook.h)
- code: HkGetEqType now differs between more equipment types (see FLHook.h)
- bug: fixed critical bugs introduced in version 1.5.7
- bug: fixed beam command
- reworked admin rights, see "RIGHTS" section in the readme
- added new admin command: getrep (<-> setrep)
- code: fixed "engine kill" detection (returned by HkGetEngineState)

1.5.7 plugin beta
=====
- included FLCoreSDK (see readme.txt in ./src/sdk/FLCoreSDK) by Skif
- added many FLHook exports (see FLHook.h in the sdk folder)
- added FLServer header files that should now be the base for further reversing development (see sdk folder)
- socket: added some security measures to further protect the socket interface from external attacks (IPs are now logged)
- socket: set the outgoing buffer to 300kb so that long messages dont get cut
- ingame chat: fixed flserver native commands "/l", "/s" and "/g", local chat is now blue (only if chatfont usercommand is enabled)
- ini: ReservedSlots, TorpMissileBaseDamageMultiplier and Debug added
- code: removed FLHookStart injection and unload features completely
- code: hooked IServerImpl::Startup so that advanced hooks can be installed prior to the actual server startup
- init: FLHook now forces the FLServer to load the complete universe before going live (fixes an autobuy bug and helps spotting errors in mods)
- log: all log files are now saved in "Freelancer/EXE/flhook_logs/" instead of the multiplayer accounts folder
- ping/loss: removed connection checking functionality and put them into an advanced connection plugin

1.5.6 plugin beta
=====
- added plugin support
- raised max. supported players to 200
- bugfix: fixed crash bug on the disconnect delay feature
- exploitfix: fixed crash exploit on the socket interface
- eventmode: deactivated basedestroy event because it can cause heavy cpu load

1.5.5 (official)
=====
- bug: rename bug fixed

1.5.4
====
- bug: /g fixed
- bug: deletechar fixed

1.5.1
=====
- ini: bans added

1.5.0
=====
- cmd: setrep added
- source: LOG_* macros removed
- source: compiles with vc6 now! (thanks to Mischa)
- bugfix: FLHookStart works on Win2k now(thx to Mischa)
- usercmd: /ignoreid, /ids, /id, /i$, /invite$ added

1.4.9
=====
- renamebug fixed: renamed chars are now shown in the account list of flserver
- bugfix: autobuy now checks if player has neccessary rep
- ini: MultiKillMessages added
- usercmd: "/set diemsgsize" added
- cmd: resetrep added
- cmd: getgroupmembers added
- ini: maximum groupsize can be altered

1.4.7
=====
- autobuy fixed
- disconnectrelay fixed
- ChangeCruiseDisruptorBehaviour fixed

1.4.6
=====
- usercmd: /autobuy added
- usercmd: /delignore may be called with multiple ids now

1.4.4
=====
- "reserved slots" added
- ini: DisconnectDelay added
- cmd: getbasestatus added
- eventmode: basedestroy added

1.4.3
=====
- usercmd: /ignore, /ignorelist, /delignore added

1.4.2 (official)
=====
- minor changes

1.4.1
=====
- kill command added
- ini: DeathMsgTextAdminKill added
- bug: security bug fixed(thx to ET90)
- fix: memory leak in getplayers/getplayerinfo fixed

1.4.0
=====
- typos in death msg fixed
- ini: new [Style] settings added
- "connect" event added
- cmd: serverinfo shows uptime
- new commands: xgetplayers/xgetplayerinfo
- ini: AntiF1 added

1.3.9
=====
- bugfix: /s bug fixed
- source-code cleanup
- anti-cheat check("Negative good-count") also when character logs in
- cmd: new feature called "shortcut"(very useful when command targets ingame player)
       see bottom of "ADMIN-COMMANDS" section for explanation
- unload automatically kicks players with moneyfixlist
- cmd: serverinfo added
- kickmsg added, you can alter the appearance of the reason in the ini
- you can now enter a reason in the kick/kickban command
- ini restructured (plz take a look at it!)
- ini: UserCmdStyle/AdminCmdStyle added to modify the appearance of ingame command-replies
- new eventmode notifications

1.3.8
=====
- flhook tries to detect corrupted charfiles and kicks player(notification will be added to flhook_kicks.log)
  if you have a charfile that still crashes the server then let me know
- ini: DisableNPCSpawns added

1.3.7
=====
- kicks get logged to flhook_kicks.log
- lossdata measuring-intervall set to 4sec
- rename bug fixed

1.3.6
=====
- cmd: rename/deletechar added
- anticheat: "selling more good than possible" detected(notification will be added to flhook_cheaters.log and player will get banned)
- ini: pingkick/losskick added
- cmd: "loss=" in getplayers now shows a reasonable result(it's an average value calculated by LossKickFrame setting)

1.3.5 (official)
=====
- bugfix: deathmsg was shown twice when "/set diemsg" was disabled

1.3.4
=====
- /u universe message bug fixed

1.3.3
=====
- rights: "setting" removed(was equal to superadmin)
- closing the flhook console will unload flhook(no more crashes)
- ini: wport option added for unicode socket connections
- ini: user commands may be enabled/disabled

1.3.2
=====
- cmd: getplayerinfo added
- cmd: getplayers shows ip/loss (not tested yet)
- beam exception leads to player kick(the cmd is save now)
- cruise disruptor behaviour may be changed(see ini)

1.3.0
=====
- IServerImpl now entirely hooked in HkIServerImpl.cpp
- cmd: getplayers shows ping

1.2.9
=====
- usercmd: "/set chatfont" added
- submitchat exception fixed?
- killmsg: death-fuse issue solved
- savechar after trade (to prevent cheating when server crashes)
- anti-baseidle added
- cmd: addcargo/removecargo/enumcargo added
- source-code cleanup
- bug: addcash/getcash now work with clientid as argument

1.2.8
=====
- ingame admin-command replies shown in a new format
- new readme.txt
- cmd: ban/unban changed
- ini changed
- print functions enhanced
- cmd: moneyfixlist added

1.2.7
=====
- bug: getcash bug(player in charmenu) fixed

1.2.6
=====
- bug: /leave bug in msg-cmd fixed 
- bug: special-char bug in socket connection fixed 
- killmsg: shows "xxx has died" when freighter with death-fuse was destroyed
- cmd: addcash/setcash now work flawlessly without kicks

1.2.5
=====
- xml format docu enhancements (however color="#..." doesnt work correctly)
- eventmode: login shows clientid
- diemsg bug fixed(player with <,> or & in their name had no die-msgs)
- cmd: isloggedin now returns false when player is in charmenu
- cmd: addcash/setcash now kick when player is logged in with a different char from same the account
- socket: bugfix, more than 1 cmd in one tcp packet now allowed (seperated by \n or \r\n)
- improved kill detection(now works with death-fuses?)

1.2.4
=====
- charfile encryption disabled

1.2.3
=====
- die-msg suppress fixed
- cmd: isloggedin added
- cmd: isonserver changed
- usercmd: /set diemsg added

1.2.2
=====
- fmsg added with xml-syntax
- eventmode: login added
- eventmode: chat spams id= and idto=
- try-catch statements added in callbacks
- release-mode-compilation finally works
- readme.txt added

