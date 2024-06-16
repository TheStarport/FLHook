---
author: "Nen"
date: "2024-06-16"
title: Commands
---

# Command Processing

As FLHook is a server only extension, we must use chat commands in order to expose functionality to the user, one being for normal players and another system for admins with permission checks. As of 5.0 FLHook will handle the command selection and parameter conversions for you.
An example function in core would be

```
void UserCommandProcessor::GiveCash(std::wstring_view characterName, std::wstring_view amount)
{
    ...stuff
}
```

There exists a global variable named userCmdClient which is only valid for the scope of the user command function, this is the clientId of the user that evoked this function.

:::{note}
These functions cannot be static or const, must be member functions of a class, and must return void. Any string parameters must be wstring_view.
:::
 
Supported types for parameters are 
 - any primitive
 - wstring_view 
 - Ship Archetype pointer (Archetype::Ship*)
 - GoodInfo pointer (GoodInfo*)
 - Equipment Archetype Pointer(Archetype::Equipment*)
 - ClientId
 - BaseId
 - SystemId
 - CharacterId
 - std::vector< std::wstring_view >
 - std::vector of numerical primitives such as floats or integers.
 

:::{note}
std::vector is greedy and therefor will capture any parameters supplied by the user after that point, Example:
"/func bullet dude 3 4 5 6 7 8"
if we have std::vector< int > as our 3rd paramater, it will consume 3-8 to construct that vector, thus it must be the last parameter and to not do so will cause a compiler error.
:::

# Command and Function matching

The actual function itself is rather straighforward, setting up the functions for user or admin commands is a bit more involved.

In order to expose functions to user command processing a plugin must define an array of Function as such

```
inline static const std::array<CommandInfo<PluginName>, N> commands =
{
    AddCommand(PluginName, Cmds(L"/foo", L"/foo2"), Foo, L"/foo", L" Insert description of your function for help command here."),
    AddCommand(PluginName, Cmds(L"/bar"), Bar, L"/bar <parameter1> <parameter2>", L" Insert description of your function for help command here."),
    ....
}
```
**N** is the number of functions you have in the array. 


AddCommand is a macro and each of the parameters are thus
- Plugin Class name,
- Cmd() macro with an array of words used to evoke the function, each word should always start with a / 
- The function itself, see above for restrictions and requirements.
- Help text string name of the function, should always match the first string in the second parameter,
- Description help text of the function, should indicate usage.

After your array of commands is properly setup, simply call this function
```
SetupUserCommandHandler(PluginName, commands);
```
And your plugin is now set up to handle user commands from FLHook.


Setting up admin commands is similar with only a few minor changes and a few additions, for example
```
 const inline static std::array<AdminCommandInfo<Plugin>, N> adminCommands = 
 {
    AddAdminCommand(PluginName, Cmds(L".foo"), Foo, GameAndConsole, Cash, L".foo",L"Insert description of your function for help command here"),
    ....
 }
```
Most of the parameters for AddAdminCommand should be familiar to User Commands with a few extras,
string used to evoke admin functions should start with . instead of / (eg: .foo instead of /foo)
4th paramater which in the example is GameAndConsole is an enum that describes the context the command is able to be executed from, as admin commands can be executed from more than just the game it is necessary to sometimes guard against using
certain commands in a context it makes no sense, such as say teleported yourself as this would require the admin to physically be in the game world and not using FLserver Console. 
The 5th parameter is a permission enum, FLHook core comes with its own enum of default roles, but you may implement your own enum as well and will check the appropriate roles from the character document.

The function requirements are largely identical to User commands, except it must return an std::wstring instead of void, the purpose of this string is to provide feedback to the admin that exectued the command either in game or from console.

And like user commands to set it up is simply
```
SetupAdminCommandHandler(Plugin, adminCommands);
```

