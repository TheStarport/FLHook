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

inline ReturnCode operator&(ReturnCode a, ReturnCode b) { return static_cast<ReturnCode>(static_cast<uint>(a) & static_cast<uint>(b)); }

enum class SpecialChatIds : uint
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

NLOHMANN_JSON_SERIALIZE_ENUM(DamageMode, {
                                             {DamageMode::None,     0},
                                             { DamageMode::PvE, "pve"},
                                             { DamageMode::PvP, "pvp"},
                                             { DamageMode::All, "all"},
});

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

NLOHMANN_JSON_SERIALIZE_ENUM(AllowedFormation,
                             {
                                 {  AllowedFormation::Alpha,   "alpha"},
                                 {   AllowedFormation::Beta,    "beta"},
                                 {  AllowedFormation::Gamma,   "gamma"},
                                 {  AllowedFormation::Delta,   "delta"},
                                 {AllowedFormation::Omicron, "omicron"},
                                 {  AllowedFormation::Omega,   "omega"},
                                 {  AllowedFormation::Sigma,   "sigma"},
                                 {AllowedFormation::Epsilon, "epsilon"},
                                 {   AllowedFormation::Zeta,    "zeta"},
                                 {  AllowedFormation::Theta,   "theta"},
                                 {  AllowedFormation::Kappa,   "kappa"},
                                 { AllowedFormation::Lambda,  "lambda"},
                                 {   AllowedFormation::Iota,    "iota"},
                                 {    AllowedFormation::Red,     "red"},
                                 {   AllowedFormation::Blue,    "blue"},
                                 {   AllowedFormation::Gold,    "gold"},
                                 {  AllowedFormation::Green,   "green"},
                                 { AllowedFormation::Silver,  "silver"},
                                 {  AllowedFormation::Black,   "black"},
                                 {  AllowedFormation::White,   "white"},
                                 { AllowedFormation::Yellow,  "yellow"},
                                 {  AllowedFormation::Matsu,   "matsu"},
                                 { AllowedFormation::Sakura,  "sakura"},
                                 {   AllowedFormation::Fuji,    "fuji"},
                                 {  AllowedFormation::Botan,   "botan"},
                                 {   AllowedFormation::Hagi,    "hagi"},
                                 { AllowedFormation::Susuki,  "susuki"},
                                 {   AllowedFormation::Kiku,    "kiku"},
                                 { AllowedFormation::Yanagi,  "yanagi"},
})
