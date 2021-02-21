# FLHook - A Server Improvement for Freelancer

## Installation
**N.B. FLHOOK ONLY WORKS WITH THE 1.1 PATCH INSTALLED. USING IT WITH 1.0 WILL CRASH FLSERVER!**

For the purpose of this readme, your Freelancer root installation folder (e.g. by default `C:\Program Files (x86)\Microsoft Games\Freelancer`) will be referred to as `.`.

1. Copy all files from `dist\$Configuration` to your `.\EXE` directory. You may selectively remove some of the default plugins as desired.
2. Edit `.\EXE\FLHook.ini` as required.
3. Edit `.\EXE\dacomsrv.ini` and append `FLHook.dll` to the `[Libraries]` section.
4. FLServer should now load FLHook whenever you start it!

## Plugin Installation

Typically, plugins are distributed as DLL files (or archives with a DLL and configuration files).
The plugin DLL needs to be in `.\EXE\flhook_plugins`. Verify with the plugin's documentation for additional setup steps.

## Configuration

All configuration options are found in `.\EXE\FLHook.ini` and are documented therein. Note that plugins may create additional options, either in this file or elsewhere. Refer to the plugin's documentation for more information.

## Administrative Commands

Commands can be executed by administrators in several ways:
* Using the FLHook console (access to all commands)
* In game by typing `.command` in the chat(e.g. `.getcash Player1`). This will only work when you own the appropriate rights which may be set via the `setadmin` command. FLHook will store the rights of each player in their account directory in the file `flhookadmin.ini`.
* Via a socket connection in raw text mode (e.g. with putty). Connect to the port given in `.\EXE\FLHook.ini` and enter `PASS password`. After having successfully logged in, you may input commands as though from the console, provided the password is associated with the requisite permissions. You may have several socket connections at the same time. Exiting the connection may be done by entering "quit" or simply by closing it.

All commands return "OK" when successful or "ERR some text" when an error occurred. A full list of commands can be found with `.help`.

### Shorthand

There are four shorthand symbols recognized by FLHook to simplify the job of selecting a character for a command.

First, you can use client IDs instead of character names by appending `$` to the command: `getcash$ 12` will perform the command for the currently logged in player with client ID 12.

You can also use "shortcuts" instead of the whole character name for a currently logged in player by appending `&` to the cmd. FLHook traverses all logged in players and checks if their character name contains the substring you passed (case insensitive) and if so, the command will operate on this player. An error will be shown if the search string given is ambiguous. For instance, given two players logged in, Foobar and Foo, `.kick& bar` will kick Foobar, but `.kick& Foo` will fail because two players match.

Third, you can target yourself by appending `!` to the command. This obviously only works if you are executing the command as a logged in player. For instance, `.setcash! 999999` will set your own character's cash to $999,999.

Finally, you can target your currently selected target by appending `?` to the command. Again, this only works if you are logged in as a player, in space, and targeting *another player*. For instance, `.kill?` will destroy whoever you are targeting.

### Permissions
Permissions may be separated by a comma (e.g. `setadmin playerxy cash,kickban,msg`). The following permissions are available by default:
* `superadmin`  → Everything
* `cash`        → Cash commands
* `kickban`     → Kick/ban commands
* `beamkill`    → Beam/kill/resetrep/setrep command
* `msg`         → Message commands
* `cargo`       → Cargo commands
* `characters`  → Character editing/reading commands
* `reputation`  → Reputation commands
* `eventmode`   → Eventmode (only when connected via socket)
* `settings`    → Rehash and reserved slots (`setadmin` only with `superadmin` rights)
* `plugins`     → Plugin commands
* `other`       → All other commands
* `special1`    → Special commands #1
* `special2`    → Special commands #2
* `special3`    → Special commands #3
All other commands except `setadmin`/`getadmin`/`deladmin` may be executed by any admin.

### XML Text Reference

The `fmsg*` commands allow you to format text in several ways (like in `.\exe\misctext.dll` entries). Text is enclosed in `<TEXT></TEXT>` tags while the format can be changed with `<TRA .../>`. Nodes names must be written in all caps! Be sure to replace the following characters within a text node:
```
< → &#60;
> → &#62;
& → &#38;
```

#### TRA node syntax

The data field of a `TRA` node consists of an RGB value along with format specifications:
```xml
<TRA data="0xBBGGRRFF" mask="-1"/>
```
* `BB` is the blue value
* `GG` is the green value
* `RR` is the red value
* `FF` is the format value

(All in hexadecimal representation)

Format flags are:
| bin        | hex  | dec   | Formatting |
|------------|------|-------|------------|
| `00000001` | `1 ` | `1  ` | Bold       |
| `00000010` | `2 ` | `2  ` | Italic     |
| `00000100` | `4 ` | `4  ` | Underline  |
| `00001000` | `8 ` | `8  ` | Big        |
| `00010000` | `10` | `16 ` | Big & wide |
| `00100000` | `20` | `32 ` | Very big   |
| `01000000` | `40` | `64 ` | Smoothest? |
| `10000000` | `80` | `128` | Smoother?  |
| `10010000` | `90` | `144` | Small      |

Simply add the flags to combine them (e.g. 7 = bold/italic/underline), and remember to represent them as a hexadecimal value.

Examples:
* `fmsgu <TRA data="0x1919BD01" mask="-1"/><TEXT>A player has died: Player</TEXT>` This is similar to the standard death message (which is shown in bold red).
* `fmsgu <TRA data="0xFF000003" mask="-1"/><TEXT>Hello</TEXT><TRA data="0x00FF0008" mask="-1"/> <TEXT>World</TEXT>` This will show "Hello World" ("Hello" will be blue/bold/italic and "World" green/big).

## Event Mode
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

## User Commands

User commands may be entered in game by every player in chat and can be enabled or disabled in the INI config. Enter them in game to get a description, and use `/help` to list all of them.

## Logging

FLhook log files are created in `.\EXE\flhook_logs`. The following logs are created:

* `flhook.log`: General debug log.
* `flhook_kicks.log`: Logs all character kicks and their cause (e.g. command, idle, ping, packetloss, corrupted character file, IP ban, host ban).
* `flhook_cheaters.log`: Cheat detection logging.
* `flhook_connects.log`: Every login attempt, if enabled.
* `flhook_usercmds.log`: Every user command, if enabled.
* `flhook_admincmds.log`: Every admin command, if enabled.
* `flhook_socketcmds.log`: Every socket command, if enabled.
* `flhook_perftimers.log`: Performance timer reports, if enabled.

If `debug` is enabled in the INI config, timestamped debug log files will be created
in  `.\EXE\flhook_logs\debug`.

## Known Issues

* The sender in the standard chat messages appear a little bit smaller when
  `/set chatfont` is enabled.

## Compiling

Starting with version 2.1, Visual Studio 2019 is required. To compile, simply build the solution as normal. The final build will be found in `dist`, whereas intermediate files are located in `int` and `bin`.

If you wish to develop a new plugin within the solution, it is highly recommended to follow the existing structure, imitate current plugins' project settings, and import the `Plugin Common` properties sheet.

If you would like the build process to copy FLHook as well as any enabled plugin, you may set the `FLHOOK_COPY_PATH` environment variable to your `.\EXE` directory. If it is not set, no such copy will occur. Unlike the `dist` folder structure, no configuration subfolder will be created, so Debug and Release builds will overwrite one another.

### Compiling Generator

The Generator project should rarely need to be recompiled since it's only used to generate some seldom-changed files. If this is necessary, some extra steps are needed to support it.

1. Set the solution to "Generator". This will suspend building other projects.
2. Follow the instructions [here](https://github.com/foonathan/cppast#installation-on-windows) to setup llvm, clang and `llvm-config`. Change the generator for cmake as required (e.g. Visual Studio 2019 instead).
3. Build solution as normal.

### Note on the STL

Freelancer was built using Visual C++ 6.0 libraries. A few functions in Freelancer's API use STL classes (e.g. `std::string`, `std::vector`, etc.). Unfortunately, binary compatibility between VC6 and modern compilers was not preserved, and as a result interoperability is not possible.

In order to circumvent this issue, FLHook (as of version 2.1) includes a minimal reimplementation of the VC6 STL called `st6`. Affected API functions have had their signatures modified to invoke `st6` instead of `std`. Most `std` features should "just work" on `st6` classes, though explicit conversion is required to switch between libraries (e.g transforming a `st6::map` into a `std::map` or vice-versa requires rebuilding the map).

### Upgrading from FLHook 2.0 and below

FLHook 2.1 includes some fairly significant differences in its handling of the STL (see above). As a result, it is likely that old plugins will need minor modifications. As of this version, `FLHookVC6Strings.dll` is *no longer necessary*, nor is having a working VC6 compiler to recompile the project. All string management-related functions were removed and can be replaced with standard `string` constructors (pointers are no longer necessary).

`Get_PluginReturnCode` was officially deprecated.

## Blowfish Encryption

Because Blowfish only supports encrypting 8 bytes of data at once, it is necessary to pad data (with `NUL`s) that is not a multiple of 8 bytes. When receiving encrypted data from FLHook, you should check for null characters and remove them from the decrypted string. When sending encrypted data to FLHook, you should pad the string with null characters if necessary. If the Blowfish implementation you are using specifically encrypts and decrypts strings, this may already be handled by it.

## Contributing and Support

Merge requests are welcome. The old Forge SVN repositories are no longer tracked and will not be merged into this repository unless the commit author also creates a merge request here.

For any and all support, please visit https://the-starport.net and look for the FLHook forums. Github issues are available for bug reports *only*.

## Visual Studio Extension

A visual studio extension is available to ease the plugin creation process. A new project template named "FLHook Plugin Template" will create a ready to use FLHook plugin project for you.

The extension is available on the visual studio marketplace here: https://marketplace.visualstudio.com/items?itemName=Alley.flhook-plugin-templates

You can also build it yourself with the project located in misc/flhook_vsix.

## Credits

* Initial FLHook development by mc_horst
* Versions 1.5.5 to 1.6.0 by w0dk4
* Versions 1.6.0 to 2.0.0 based on open-source SVN, supervised by w0dk4
* Versions 2.1.0 and later on Github, supervised by FriendlyFire

Special thanks to w0dk4 and Niwo for testing and the whole Hamburg City Server admin team for the suggestions and feedback.

FLHook uses a slightly modified version of `flcodec.c`.
