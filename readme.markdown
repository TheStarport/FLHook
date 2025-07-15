# FLHook - A Server Improvement for Freelancer
[![Build](https://github.com/fl-devs/FLHook/actions/workflows/release.yml/badge.svg)](https://github.com/fl-devs/FLHook/releases/latest/download/Release.zip) [![Discord](https://badgen.net/badge/icon/discord?icon=discord&label)](https://discord.com/invite/c6wtsBk) [![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/fl-devs/FLHook/graphs/commit-activity) [![Website shields.io](https://img.shields.io/website-up-down-green-red/http/shields.io.svg)](https://flhook.org/)


[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=fl-devs_FLHook&branch=master&metric=bugs)](https://sonarcloud.io/dashboard?id=fl-devs_FLHook&branch=master)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=fl-devs_FLHook&branch=master&metric=code_smells)](https://sonarcloud.io/dashboard?id=fl-devs_FLHook&branch=master)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=fl-devs_FLHook&branch=master&metric=duplicated_lines_density)](https://sonarcloud.io/dashboard?id=fl-devs_FLHook&branch=master)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=fl-devs_FLHook&branch=master&metric=ncloc)](https://sonarcloud.io/dashboard?id=fl-devs_FLHook&branch=master)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=fl-devs_FLHook&branch=master&metric=alert_status)](https://sonarcloud.io/dashboard?id=fl-devs_FLHook&branch=master)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=fl-devs_FLHook&branch=master&metric=security_rating)](https://sonarcloud.io/dashboard?id=fl-devs_FLHook&branch=master)

## What is FLHook?
FLHook is a server-side extension utility for Freelancer (2003) that proviudes a framework for running custom commands
and code that can manipulate player and server data. It uses a plugin based architecture and ships with a collection
of examples plugins and commands.

## Documentation
The docs can be found [here](https://flhook.the-starport.com) or can be built locally from the repo. See [here](#building) for more information.

## Usage
**N.B. FLHOOK ONLY WORKS WITH THE 1.1 PATCH INSTALLED. USING IT WITH 1.0 WILL CRASH FLSERVER!**

In order to use FLHook, a few dependencies are required:
- [MongoDB Community Server](https://www.mongodb.com/try/download/community)
- [Visual Studio Redistributables (x86)](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170)

You can grab the latest FLHook release from [here](https://github.com/TheStarport/FLHook/releases/latest). 

Freelancer's directory with FLHook structure is as follows:
```
Freelancer/
├── DATA/
│   └── ....
├── DLLS/
│   └── ....
└── EXE/
    ├── ....
    ├── plugins/
    │   ├── arena.dll
    │   └── etc...
    ├── config/
    │   ├── arena.json
    │   └── etc...
    ├── dacomsrv.ini
    ├── FLServer.exe
    └── FLHook.dll
```

When copying the contents of the release build to your Freelancer directory, ensure that it matches the above diagram.
After doing the copy you **must** edit "dacomsrv.ini" and add "FLHook.dll" to the bottom of the "Libraries" section.

After doing so, if everything is correct, when you launch FLServer.exe you should be greeted with a console window that
welcomes you to FLHook and lists all your loaded plugins. This console will allow you to run various commands once the 
the console has given you the "FLHook Ready" message.

Type `help` in the console to see your options. For further information, see the [documentation](https://flhook.the-starport.com).

## Configuration

All configuration options are found in `.\EXE\FLHook.json` and are documented therein. Note that plugins may create additional options, either in this file or elsewhere. Refer to the plugin's documentation for more information.

## Cloning

When cloning ensure that you run `git clone` with `--recurse-submodules`, otherwise you will run into compiler errors.
If you have already cloned it and didn't run the above option, open a terminal in the cloned directory and run the 
following command:

```
git submodule update --init
```

## Building

FLHook has the following requirements to be built:
- Visual Studio 2022 (vc143)
- Python 3.11+
- cmake
- Doxygen (for building docs)

It's recommended that you use a Python virtual environment for FLHook. This can be done with the following commands:

```ps
python -m venv .venv
./.venv/Scripts/activate.ps1
```

### Building the project and plugins

Your first build from a fresh clone will require the below commands:
```
pip install -r requirements.txt
python cli.py configure
```
If, whilst running the `pip install` command, you receive warnings about the Scripts directory (which pip will have
written conan to) not being on your path, open the Start Menu and run `Edit environment variables for your account`,
then proceed to the configure step.
If, whilst running the `python cli.py configure` command, you receive `[ERROR:vcvars.bat] Toolset directory for version
'14.4' was not found.`, you may need to update your copy of Visual Studio 2022.

After that first build, an IDE of choice can be used. This command will use our package manager Conan2 to download and
build all needed dependencies and setup the cmake project for usage. Most IDEs can then load the folder and run the
generated CMake configuration.
- JetBrains CLion can handle this all natively once the folder is open
- VSCode requires two plugins, CMake and cmake-tools in order to use cmake probably.
- Visual Studio supports it out of the box, via "File" -> "Open" -> "CMake...", and select the CMakeLists.txt file in
  the FLHook directory you cloned.

Alternatively, `python cli.py build` may be used.

### Building Docs

Building the docs can be done with the following commands
```ps
pip install -r requirements.txt
doxygen Doxyfile
cd docs
sphinx-build -M html . ./_build
python -m http.server 8080 -d /_build/html
```

## Contributing

Pull requests are welcome, but be aware that this project ships with a .clang-format file. This file should be treated
as the style guide and code should generally conform to how it formats (with exceptions).

For casing we use `camelCase` in the following instances:
- Variable names
- Class/struct fields
- Function parameters

We use `PascalCase` in the following instances:
- Function Defintions
- Using/Typedef staements
- Class/Struct definitions
- Template definitions

Macros can be defined either as `SCREAMING_SNAKE_CASE` or as `PascalCase`, depending on the context.

## Credits

* Initial FLHook development by mc_horst.
* Versions 1.5.5 to 1.6.0 by w0dk4.
* Versions 1.6.0 to 2.0.0 based on open-source SVN, supervised by w0dk4.
* Versions 2.1.0 and later on Github, supervised by FriendlyFire.
* Versions 4.0.0 and later on Github, supervised by Laz and Raikkonen.
* Versions 5.0.0 and later on Github, supervised by Laz and Aingar

*Version 3.0.0 was skipped due to conflicts with the Discovery mod*
