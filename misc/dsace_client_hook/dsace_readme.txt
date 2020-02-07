DSAce.dll is a client side hook used primarily by discovery based mods and works in conjunction with the FLHook plugin dsacesrv.dll. This hook provides additional functionality for Freelancer including limited client side anticheat, infocard and economy updates under server control, turret zoom, cloaking and jumpdrive support.

You need to use this hook if you use the player owned base plugin (base.dll), the mobile docking plugin (dock.dll), or the cloaking plugin (cloak.dll) or the jump functionality in the player control plugin. This plugins send messages to the client hook either directly embedded in hidden chat messages or via the dsacesrv flhook plugin.

To use this plugin, add DSAce.dll to the [Libraries] section of dscom.ini.

The source code is available to limited people. Ask if you would like a full copy. Non-sensitive parts of the code are included in this folder in the files public_funcs.cpp and equip_funcs.cpp.

The anticheat functions added in this plugin are not very good but are better than nothing.

-- Cannon 19 Sep 2012