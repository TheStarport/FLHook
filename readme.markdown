# FLHook - A Server Improvement for Freelancer
[![Build](https://github.com/fl-devs/FLHook/actions/workflows/release.yml/badge.svg)](https://github.com/fl-devs/FLHook/releases/latest/download/Release.zip) [![Discord](https://badgen.net/badge/icon/discord?icon=discord&label)](https://discord.com/invite/c6wtsBk) [![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/fl-devs/FLHook/graphs/commit-activity) [![Website shields.io](https://img.shields.io/website-up-down-green-red/http/shields.io.svg)](https://flhook.org/)


## Installation
**N.B. FLHOOK ONLY WORKS WITH THE 1.1 PATCH INSTALLED. USING IT WITH 1.0 WILL CRASH FLSERVER!**

For the purpose of this readme, your Freelancer root installation folder (e.g. by default `C:\Program Files (x86)\Microsoft Games\Freelancer`) will be referred to as `.`.

1. Copy all files from `dist\$Configuration` to your `.\EXE` directory. You may selectively remove some of the default plugins as desired.
2. Edit `.\EXE\dacomsrv.ini` and append `FLHook.dll` to the `[Libraries]` section.
3. Launch `FLServer.exe`, this should generate a fresh `FLHook.json` file.
4. Edit this file as needed and relaunch `FLServer.exe`.

## Plugin Installation

Typically, plugins are distributed as DLL files (or archives with a DLL and configuration files).
The plugin DLL needs to be in `.\EXE\flhook_plugins`. Verify with the plugin's documentation for additional setup steps.

## Configuration

All configuration options are found in `.\EXE\FLHook.json` and are documented therein. Note that plugins may create additional options, either in this file or elsewhere. Refer to the plugin's documentation for more information.

## User Commands

User commands may be entered in game by every player in chat and can be enabled or disabled in the JSON config. Enter them in game to get a description, and use `/help` to list all of them.

## Compiling

Starting with version 2.1, Visual Studio 2019 is required. To compile, simply build the solution as normal. The final build will be found in `dist`, whereas intermediate files are located in `int` and `bin`.

If you wish to develop a new plugin within the solution, please see our `Plugin Development` section in the [docs](https://docs.flhook.org).

If you would like the build process to copy FLHook as well as any enabled plugin, you may set the `FLHOOK_COPY_PATH` environment variable to your `.\EXE` directory. If it is not set, no such copy will occur. Unlike the `dist` folder structure, no configuration subfolder will be created, so Debug and Release builds will overwrite one another.

### Compiling Instructions

1. Install Visual Studio 2019
2. Clone this repository (`git clone --recursive https://github.com/fl-devs/FLHook.git`)
3. Run `project\vcpkg.bat`
4. Open `project\FLHook.sln`
5. Ensure you are on `Release` (or `ReleaseWithDebug` for development) and build the solution.
6. Build Solution. vcpkg should download a number of dependencies.

## Contributing and Support

Merge requests are welcome. The old Forge SVN repositories are no longer tracked and will not be merged into this repository unless the commit author also creates a merge request here.

For any and all support, please join the [Discord](https://discord.com/invite/c6wtsBk) or visit the [forums](https://the-starport.net) and look for the FLHook section. Github issues are available for bug reports and feature requests *only*.

## Credits

* Initial FLHook development by mc_horst.
* Versions 1.5.5 to 1.6.0 by w0dk4.
* Versions 1.6.0 to 2.0.0 based on open-source SVN, supervised by w0dk4.
* Versions 2.1.0 and later on Github, supervised by FriendlyFire.
* Versions 4.0.0 and later on Github, supervised by Laz and Raikkonen.

Special thanks to:
* FriendlyFire for kicking off the initial rewrite.
* Nen for testing and plugin development.
* All those who have worked on FLHook before.

FLHook uses a slightly modified version of `flcodec.c`.
