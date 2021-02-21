#pragma once

enum class HookedCall__IEngine {
    CShip__Init,
    CShip__Destroy,
    UpdateTime,
    ElapseTime,
    DockCall,
    LaunchPosition,
    ShipDestroyed,
    BaseDestroyed,
    GuidedHit,
    AddDamageEntry,
    DamageHit,
    AllowPlayerDamage,
    SendDeathMessage
};

enum class HookedCall__FLHook {
    TimerCheckKick,
    TimerNPCAndF1Check,
    PluginCommunication,
    UserCommand__Help,
    UserCommand__Process,
    AdminCommand__Help,
    AdminCommand__Process,
    LoadSettings,
    LoadCharacterSettings,
    ClearClientInfo,
    ProcessEvent
};

enum class HookedCall__IChat {
    SendChat
};