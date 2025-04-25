# Changelog

## 5.0
FLHook 5.0 is a total rewrite compared to 4.0, with no file left unchanged in the process. Main changes and new features include:
- Replace Vanilla character save format from .ini files to MongoDB powered solution.
- Rewritten Plugin architecture to be object oriented. All plugins will need to be converted.
- Game data access is abstracted away into a collection of IDs with member functions indicating what they are for.
- Built in FLServer optimizer.
- Documentation rewritten in a new format (work in progress).
- Built in spawning API for ships and solars
- Added Resource Manager, which tracks spawned entities.
- FLHook class centralizes configs and resource access.
- Added an asynchronous task system, FLHook core uses it for database actions.
- Added Infocard Manager to look up infocards.
- Replaced old Socket system with a HTTP server extension.
- Mail moved to Core, and its now a API level feature, letting you send persisten messages to offline  players.
- Added Personality Helper to look up Personality AI (pilot) data.
- Added API for looking up in-game hashes (InternalAPI::HashLookup).
- Added the ability to manually toggle NPC spawns via InternalAPI::ToggleNPCSpawn() call (previously automatic based on server load).
- Added API for generating random numbers and strings.
- Added API for reading/writing JSON, used for plugin/core configs.
- Command system rewritten to automaticallly type convert parameters into FLHook abstracted versions allowing for easy manipulation via member functions. (See example plugins)
- Redone logger to support writing to JSON.
- Refactored old CrashCatcher plugin and integrated it into core.
- NPC detection and despawn ranges now configurable.
- Customization of Save Location. (only used for server metadata, as player data is now stored in a database).
- Created Utility for converting vanilla player data into the database format.

TODO:
- Finish FLAdmin using the HTTP server extension.
- Doublecheck Loot spawning API.
- Add changing/adding infocards to Infocard Manager.

## 4.0.33
- Changes to ensure that npc.dll can't cause a crash when attempting to spawn an invalid NPC entry.

## 4.0.32
- Properly expose data for CreateUserDefinedSolarFormation() so objects spawned this way can be tracked by other plugins.

## 4.0.31
- Fixed a case in Advanced Startup Solars where randomly placed solars could occupy the same position

## 4.0.30
- Adjusted the project file to ensure that Advanced Startup Solars is properly included in the build

## 4.0.29
- Advanced Startup Solars: Finer control over solars that spawn on startup, requires solar.dll and npc.dll to function
- Fix the IPC interface for `CreateUserDefinedSolarFormation()` in solar control

## 4.0.28
- Improve checks for spawning NPCs
- Implement 'Solar Control': Spawn solars and formations of solars via admin command.

## 4.0.27
- Improved functionality of loot tables plugin: weights & drop counts are now more customisable, reverted to weight based system instead of percentages.

## 4.0.26
- Add checks for NPC defined in npcInfo. If you define a shipArch, reputation or pilot that's invalid, the FLHook console will log an error.
- Add a check to ensure that invalid NPCs cannot be spawned via startupNpcs, which would cause the server to crash.

## 4.0.25
- Fixed some major issues with daily_tasks.

## 4.0.24
- Add checks for NPC defined in npcInfo. If you define a shipArch, reputation or pilot that's invalid, the FLHook console will log an error.
- Add a check to ensure that invalid NPCs cannot be spawned via startupNpcs, which would cause the server to crash.

## 4.0.23
- Fixed some major issues with daily_tasks.

## 4.0.22
- Add call to Release() after a CObject::Find that was causing poor performance.

## 4.0.21
- Create 'daily_tasks' a plugin that assigns basic daily tasks to player accounts for them to complete.

## 4.0.20
- Fixed issue where repair cost was not fetched from base inis.
- Fixed CargoDrop and Condata that wouldn't compile as a result of a breaking SDK change.

## 4.0.19
- Updated the SonarScan to use version 17 of Java, as 11 is no longer working and causes the pipeline to fail.
- Updated SonarScan to 5.0.1.3006

## 4.0.18
- Changed IServerImpl::RequestBestPath hook to use new BestPathInfo struct
- Added Hk::Player::SendBestPath function which sets the waypoints of a player

## 4.0.17
- Provided configs for 0x86AEC, 0x84018, 0xD3D6E and 0x58F46 in server.dll in crash catcher. These offsets were hardcoding several values that are often changed by mods manually relating to NPC spawn and scanner range. 0 values disables each respective hook.

## 4.0.16
- Removed a PrintUserCmd from Autobuy that was printing on every repair.

## 4.0.15
- Fixed a typo in the Autobuy plugin header that caused incorrect configs to be generated.

## 4.0.14
- Fix the description of a release created by the build pipeline.

## 4.0.13
- Addressed to-do in Autobuy: adhere to Ammo Limits specified in .ini files.

## 4.0.12
- Fix the bugs reported in SonarCloud. This is mainly throwing a proper exception rather than just a string.

## 4.0.11
- Added Loot Tables plugin that allows you greater control over the loot dropped by NPCs.

## 4.0.10
- Fixed bug in CI that prevented Sonar running if there hasn't been a commit for a while.

## 4.0.9
- Fixed bug in Stats plugin where data was not being converted to UTF-8 causing issues with non-standard characters.

## 4.0.8
- Fix bugs with NPC Control Plugin.
- Fix bug with the chase admin command.

## 4.0.7
- Fix code smells shown in SonarLint.

## 4.0.6

- Moved multikill tracking to KillTracker plugin & removed corresponding functionality
from other places in the code.

## 4.0.5

- Changed CargoDrop plugin to accept more than two commodities, amount of Hull drops now calculcated by mass rather than ship hold size.

## 4.0.4

- Update autobuy to cover miscellaneous ammo types if they are present (HP_GUN).

## 4.0.3

- Fixed exceptions on JumpInComplete hook due to improper parameter count.

## 4.0.2

- Added functionality to warehouse to list all bases a player has stored items and the quantity. 
- Fixed comparison operator with withdrawing items from storage to allow user to fill cargo to full.

## 4.0.1

- Fix commands being ignored if they begin with s, g, or l.

## 4.0.0

--- TODO: Write comprehensive list of changes for 4.0.0
