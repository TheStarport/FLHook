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