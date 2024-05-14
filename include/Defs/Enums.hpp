#pragma once

enum class LogType : int
{
    Normal,
    Cheater,
    Kick,
    Connects,
    AdminCmds,
    UserLogCmds,
    PerfTimers
};

enum class DieMsgType
{
    All = 0,
    System = 1,
    None = 2,
    Self = 3,
};

enum class ChatSize
{
    Default = 0,
    Small = 1,
    Big = 2,
};

enum class ChatStyle
{
    Default = 0,
    Bold = 1,
    Italic = 2,
    Underline = 3,
};

enum class EngineState
{
    NotInSpace,
    Cruise,
    Thruster,
    Engine,
    Killed,
    Tradelane
};

enum class EquipmentType
{
    Gun,
    Torpedo,
    Cd,
    Missile,
    Mine,
    Cm,
    ShieldGen,
    Thruster,
    ShieldBattery,
    Nanobot,
    Munition,
    Engine,
    Other,
    Scanner,
    Tractor,
    Light
};

enum class PluginMajorVersion
{
    Undefined = -1,
    // We started doing this from 4 onwards
    V04 = 4,
    V05 = 5,
};

// Define most ahead of time
enum class PluginMinorVersion
{
    Undefined = -1,
    V00 = 0,
    V01,
    V02,
    V03,
    V04,
    V05,
    V06,
    V07,
    V08,
    V09,
};

enum class ReturnCode
{
    Default = 0,
    SkipPlugins = 1,
    SkipFunctionCall = 2,
    SkipAll = SkipPlugins | SkipFunctionCall,
};

inline ReturnCode operator&(ReturnCode a, ReturnCode b) { return static_cast<ReturnCode>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b)); }

enum class SpecialChatIds : unsigned int
{
    Console = 0,

    PlayerMin = 1,
    PlayerMax = 249,

    SpecialBase = 0x10000,
    Universe = SpecialBase | 0,
    System = SpecialBase | 1,
    Local = SpecialBase | 2,
    Group = SpecialBase | 3,
    GroupEvent = SpecialBase | 4
};

enum class IdType
{
    Client,
    Ship = 1,
    Solar = 1,
    Equip,
    Arch,
    Reputation
};

enum class DamageMode
{
    None,
    PvE = 1 << 0,
    PvP = 1 << 1,
    All = PvE | PvP
};

inline DamageMode operator|(DamageMode c1, DamageMode c2) { return static_cast<DamageMode>(static_cast<int>(c1) | static_cast<int>(c2)); }

enum class AllowedFormation
{
    Alpha = 1,
    Beta,
    Gamma,
    Delta,
    Epsilon,
    Zeta,
    Theta,
    Iota,
    Kappa,
    Lambda,
    Omicron,
    Sigma,
    Omega,
    Red,
    Blue,
    Gold,
    Green,
    Silver,
    Black,
    White,
    Yellow,
    Matsu,
    Sakura,
    Fuji,
    Botan,
    Hagi,
    Susuki,
    Kiku,
    Yanagi
};
