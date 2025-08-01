#pragma once

//TODO: CShip, CLoot, CSolar.

enum class CShipVTable
{
    IntializeInstance,
    DoSimulationController,
    DestroyInstance,
    SetPosition,
    GetPosition,
    SetOrientation,
    GetTransform,
    SetTransform,
    GetTransform2,
    GetCenteredRadius,
    SetCenteredRadius,
    SetInstanceFlags,
    GetInstanceFlags,
    WriteFile,
    Destructor, // VTable has it as sub_62881E10, related to ship being cleaned up
    GetVelocity,
    SetVelocity,
    GetAngularVelocity,
    SetVelocity2,
    Open,
    Update,
    GetVelocity2,
    GetAngularVelocity2,
    DisableControllers,
    EnableControllers,
    GetPhysicalRadiusR,
    GetCenterOfMass,
    GetMass,
    GetSurfaceExtents,
    UnMakePhysical,
    RemakePhysical,
    BeamObject,
    InitCSimple,
    CachePhysicalProperties,
    GetName,
    IsTargetable,
    Connect,
    Disconnect,
    SetHitPoints,
    InitPhysics,
    InitCObject,
    LoadEquipAndCargo,
    ClearEquipAndCargo,
    GetEquipDescList,
    AddItem,
    Activate,
    GetActivateState,
    Disconnect2,
    Disconnect3,
    Connect2,
    Notify,
    FlushAnimations,
    GetTotalHitPoints,
    GetTotalMaxHitPoints,
    GetTotalRelativeHealth,
    GetSubObjectVelocity,
    GetSubObjectCenterOfMass,
    GetSubObjectHitPoints,
    GetSubObjectHitPoints2,
    GetSubObjectMaxHitPoints,
    GetSubObjectRelativeHealth,
    InstToSubObjectId, //inst = instance?
    SubObjectIdToInst,
    DestroySubObjects,
    EnumerateSubObjects,
    AllocEquip,
    LinkShields,
    InitCShip,
    Size = InitCShip,
    Start = 0x13C02C,
    End = 0x13C138
};

//Note CLoot VTable seems to have this after the initCLoot, not sure if its random or actually apart of the VTable
//.rdata:0639D8C0                 dd offset unk_63A7F38

enum class CLootVTable
{
    InitializeInstance,
    DoSimulationController,
    DestroyInstance,
    SetPosition,
    GetPosition,
    SetOrientation,
    GetTransform,
    SetTransform,
    GetTransform2,
    GetCenteredRadius,
    SetCenteredRadius,
    SetInstanceFlags,
    GetInstanceFlags,
    WriteFile,
    Destructor, //.rdata:0639D854                 dd offset sub_629D800
    GetVelocity,
    SetVelocity,
    GetAngularVelocity,
    SetVelocity2,
    Open,
    Update,
    GetVelocity2,
    GetAngularVelocity2,
    Unload,
    Unload2,
    GetPhysicalRadius,
    GetcenterOfMass,
    GetMass,
    GetSurfaceExtents,
    UnmakePhysical,
    RemakePhysical,
    BeamObject,
    InitCSimple,
    CachePhysicalProperties,
    GetName,
    IsTargetable,
    Connect,
    Disconnect,
    SetHitPoints,
    InitPhysics,
    InitCLoot,
    Start = 0x13D81C,
    End = 0x13D8C0
};

enum class CSolarVTable
{
    InitializeInstance,
    DoSimulationController,
    DestroyInstance,
    SetPosition,
    GetPosition,
    SetOrientation,
    GetTransform,
    SetTransform,
    GetTransform2,
    GetCenteredRadius,
    SetCenteredRadius,
    SetInstanceFlags,
    GetInstanceFlags,
    WriteFile,
    Destructor, //.rdata:0639D3FC                 dd offset sub_629AC10
    GetVelocity,
    SetVelocity,
    GetAngularVelocity,
    SetVelocity2,
    Open,
    Update,
    GetVelocity2,
    GetAngularVelocity2,
    DisableControllers,
    EnableControllers,
    GetPhysicalRadius,
    GetCenterOfMass,
    GetMass,
    GetSurfaceExtents,
    UnmakePhysicals,
    RemakePhysicals,
    BeamObject,
    InitCSolar,
    CachePhysicalProps,
    GetName,
    IsTargetable,
    Connect,
    Disconnect,
    SetHitPoints,
    InitPhysics,
    InitCEquipObject,
    LoadAndEquipAndCargo,
    ClearEquipAndCargo,
    GetEquipDescriptionList,
    AddItem,
    Activate,
    GetActiveState,
    Disconnect2,
    Disconnect3,
    Connect2,
    Notify,
    FlushAnimations,
    GetTotalHitPoints,
    GetTotalMaxhitPoints,
    GetTotalRelativeHealth,
    GetSubObjectVelocity,
    GetSubObjectCenterOfMass,
    GetSubOjectHitPoints,
    GetSubOjectHitPoints2,
    GetSubObjectMaxHitPoints,
    GetSubObjectRelativeHealth,
    InstToSubObjectId,
    SubObjectIdToInst,
    DestroySubObjects,
    EnumerateSubObjects,
    AllocEquip,
    LinkShields,
    Start = 0x13D3C4,
    End = 0x13D4CC
};

enum class CGuidedVTable
{
    InitializeInstance,
    DoSimulationController,
    DestroyInstance,
    SetPosition,
    GetPosition,
    SetOrientation,
    GetTransform,
    SetTransform,
    GetTransform2,
    GetCenteredRadius,
    SetCenteredRadius,
    SetInstanceFlags,
    GetInstanceFlags,
    WriteFile,
    Destructor, //.rdata:0639BCA4                 dd offset sub_628630
    GetVelocity,
    SetVelocity,
    GetAngularVelocity,
    SetVelocity2,
    Open,
    Update,
    GetVelocity2,
    GetAngularVelocity2,
    DisableControllers,
    EnableControllers,
    GetPhysicalRadius,
    GetCenterOfMass,
    GetMass,
    GetSurfaceExtents,
    UnmakePhysicals,
    RemakePhysicals,
    BeamObject,
    InitCSimple,
    CachePhysicalProps,
    GetName,
    IsTargetable,
    Connect,
    Disconnect,
    SetHitPoints,
    InitPhysics,
    InitCEquipObject,
    SetDead,
    ExpireSafeTime,
    Start = 0x13BC6C,
    End = 0x13BD14
};

enum class CELauncherVTable
{
    IsTriggered,
    IsDestroyed,
    IsFunctioning,
    IsDisabled,
    IsTemporary,
    CanDelete,
    NotifyArchGrpDestroyed,
    IsLootable,
    Update,
    GetEquipDesc,
    GetStatus,
    Activate,
    Destroy,
    GetMaxHitPoints,
    GetHitPoints,
    SetHitPoints,
    GetRelativeHealth,
    GetConnectionPosition,
    IsControllerEnabled,
    EnableController,
    DisableController,
    IsConnected,
    Connect,
    GetParentHpName,
    GetParentConnector,
    GetHardPointInfo,
    GetVelocity,
    GetCenterOfMass,
    Disconnect,
    GetToughness,
    GetRadius,
    IsInstOnEquip,
    GetRootIndex,
    SetFate,
    ComputeBoundingSphere,
    Fire,
    ConsumeFireResources,
    ComputeLaunchInfo,
    GetPowerDrawPerFire,
    GetAmmoCount,
    AmmoNeedsMet,
    ComputeLaunchInfo_OneBarrel,
    CanFire,
    PowerNeedsMet,
    DrawPower,
    DrawAmmoFromCargo,
    RefireDelayElapsed,
    Size = RefireDelayElapsed,
    Start = 0x13CEE4,
    End = 0x13CFA0
};

enum class IShipInspectVTable
{
    GetPosition,
    GetVelocity,
    GetAngularVelocity,
    GetOrientation,
    GetTransform,
    GetCenterOfMass,
    GetSubObjCenterOfMass,
    GetIndex,
    GetId,
    GetGoodId,
    GetArchetypeExtents,
    GetPhysicalRadius,
    GetMass,
    IsTargetable,
    IsDying,
    GetStatus,
    GetStatus2,
    GetShieldStatus,
    GetShieldStatus2,
    GetThrottle,
    GetAxisThrottle,
    GetNudgeVec,
    GetStrafeDir,
    IsCruiseActive,
    IsCruiseActive2,
    AreThrustersActive,
    GetAttitudeTowards,
    GetAttitudeTowardsSymmetrical,
    GetReputation,
    GetTarget,
    GetSubtarget,
    GetSubtargetCenterOfMass,
    GetRank,
    GetAffiliation,
    GetType,
    GetBase,
    GetDockTarget,
    GetPower,
    GetZoneProps,
    GetScannerInterference,
    GetHoldLeft,
    EnumerateCargo,
    GetData,
    GetFormationOffset,
    GetFormationLeader,
    GetFollowOffset,
    GetFollowOrder,
    IsPlayer,
    GetHardpoint,
    HasDockHardpoints,
    GetDockHardpoints,
    GetTimeToAccelerate,
    GetDistanceTravelled,
    GetProjectedThrottle,
    GetSpeed,
    GetInitialSpeedToCoastDistance,
    GetTimeToAccelerateAngularly,
    GetTimeToAccelerateAngularly2,
    GetAngularDistanceTravelled,
    GetAngularDistanceTravelled2,
    GetAngularSpeedXY,
    GetAngularSpeedZ,
    GetProjectedAxisThrottleXY,
    GetProjectedAxisThrottleZ,
    GetMaxBankAngle,
    GetScanList,
    GetTargetLeadFirePosition,
    IsPointingAt,
    CanPointAt,
    FindEquipment,
    GetEquipmentStatus,
    GetEquipmentVal,
    ScanCargo,
    EnumerateSubtargets,
    GetLaneDirection,
    GetRingSide,
    TraverseRings,
    IsUsingTradelane,
    GetLaneStart,
    GenerateFollowOffsets,
    GetAtmosphereRange,
    GetToughness,
    GetBehaviorId,
    GetFormationFollowers,
    CObject,
    ObjectType,
    ObjectDestroyed,
    Disconnect,
    GetDunno0x40,
    InstantiateCObject,
    SetCObjectById,
    Update,
    sub_6D01A60,
    sub_6CEE810,
    sub_6CEE980,
    sub_6CE9250,
    MunitionImpact,
    ProcessExplosionDamage,
    sub_6D01A10,
    get_dunno_0x41,
    sub_6CEEFA0,
    sub_6CEF0F0,
    IsPlayerVulnerable,
    IsInvulnerable,
    GetMaxHpLoss,
    ProcessCollisionUnk,
    SetHpAndColGrpUnk,
    SetRelativeHealth,
    DamageHull,
    ApplyDamageEntry,
    CanDealDamage,
    sub_6CE7D00,
    sub_6CE8C50,
    sub_6CE8D40,
    sub_6CE8CD0,
    sub_6CE8E90,
    sub_6CE8E10,
    DropAllCargo,
    HitShieldBubble,
    sub_6CE88D0,
    sub_6CE8930,
    LightFuse,
    UnlightFuse,
    FuseExpirationCheck,
    DeathExplosion,
    sub_6D01C90,
    sub_6CE8760,
    ProcessExplosionDamageHull,
    ProcessExplosionDamageExtEqAndColGrps,
    ProcessExplosionDamageShieldBubble,
    ProcessExplosionDamageEnergy,
    ShieldMunitionHit,
    DamageExtEq,
    DamageShield,
    DamageColGrp,
    DamageEnergy,
    ColGrpDeath,
    CEquipDeath,
    UnkDeath,
    DealColGrpLinkedDamage,
    sub_6CEAEA0_unk,
    AIReactToHullDmg,
    AIReactToEquipDmg,
    AIReactToEnergyDmg,
    AIReactToColGrpDmg,
    sub_6CEBE80,
    sub_6CEC260,
    ProcessPerishableCargo,
    Size = ProcessPerishableCargo,
    Start = 0x8711C,
    End = 0x87368
};

enum class ISolarInspectVTable
{
    GetPosition,
    GetVelocity,
    GetAngularVelocity,
    GetOrientation,
    GetTransform,
    GetCenterOfMass,
    GetSubObjCenterOfMass,
    GetIndex,
    GetId,
    GetGoodId,
    GetArchetypeExtents,
    GetPhysicalRadius,
    GetMass,
    IsTargetable,
    IsDying,
    GetStatus,
    GetStatus2,
    GetShieldStatus,
    GetShieldStatus2,
    GetThrottle,
    GetAxisThrottle,
    GetNudgeVec,
    GetStrafeDir,
    IsCruiseActive,
    IsCruiseActive2,
    AreThrustersActive,
    GetAttitudeTowards,
    GetAttitudeTowardsSymmetrical,
    GetReputation,
    GetTarget,
    GetSubtarget,
    GetSubtargetCenterOfMass,
    GetRank,
    GetAffiliation,
    GetType,
    GetBase,
    GetDockTarget,
    GetPower,
    GetZoneProps,
    GetScannerInterference,
    GetHoldLeft,
    EnumerateCargo,
    GetData,
    GetFormationOffset,
    GetFormationLeader,
    GetFollowOffset,
    GetFollowOrder,
    IsPlayer,
    GetHardpoint,
    HasDockHardpoints,
    GetDockHardpoints,
    GetTimeToAccelerate,
    GetDistanceTravelled,
    GetProjectedThrottle,
    GetSpeed,
    GetInitialSpeedToCoastDistance,
    GetTimeToAccelerateAngularly,
    GetTimeToAccelerateAngularly2,
    GetAngularDistanceTravelled,
    GetAngularDistanceTravelled2,
    GetAngularSpeedXY,
    GetAngularSpeedZ,
    GetProjectedAxisThrottleXY,
    GetProjectedAxisThrottleZ,
    GetMaxBankAngle,
    GetScanList,
    GetTargetLeadFirePosition,
    IsPointingAt,
    CanPointAt,
    FindEquipment,
    GetEquipmentStatus,
    GetEquipmentVal,
    ScanCargo,
    EnumerateSubtargets,
    GetLaneDirection,
    GetRingSide,
    TraverseRings,
    IsUsingTradelane,
    GetLaneStart,
    GenerateFollowOffsets,
    GetAtmosphereRange,
    GetToughness,
    GetBehaviorId,
    GetFormationFollowers,
    CObject,
    ObjectType,
    ObjectDestroyed,
    Disconnect,
    GetDunno0x40,
    InstantiateCObject,
    SetCObjectById,
    Update,
    sub_6D01A60,
    sub_6CEE810,
    sub_6CEE980,
    sub_6CE9250,
    MunitionImpact,
    ProcessExplosionDamage,
    sub_6D01A10,
    get_dunno_0x41,
    sub_6CEEFA0,
    sub_6CEF0F0,
    IsPlayerVulnerable,
    IsInvulnerable,
    GetMaxHpLoss,
    ProcessCollisionUnk,
    SetHpAndColGrpUnk,
    SetRelativeHealth,
    DamageHull,
    ApplyDamageEntry,
    CanDealDamage,
    sub_6CE7D00,
    sub_6CE8C50,
    sub_6CE8D40,
    sub_6CE8CD0,
    sub_6CE8E90,
    sub_6CE8E10,
    sub_6CE8F50,
    HitShieldBubble,
    sub_6CE88D0,
    sub_6CE8930,
    LightFuse,
    UnlightFuse,
    FuseExpirationCheck,
    DeathExplosion,
    sub_6D01C90,
    sub_6CE8760,
    ProcessExplosionDamageHull,
    ProcessExplosionDamageExtEqAndColGrps,
    ProcessExplosionDamageShieldBubble,
    ProcessExplosionDamageEnergy,
    ShieldMunitionHit,
    DamageExtEq,
    DamageShield,
    DamageColGrp,
    DamageEnergy,
    ColGrpDeath,
    CEquipDeath,
    UnkDeath,
    DealColGrpLinkedDamage,
    sub_6CEAEA0_unk,
    AIReactToHullDmg,
    AIReactToEquipDmg,
    AIReactToEnergyDmg,
    AIReactToColGrpDmg,
    sub_6CEBE80,
    sub_6CEC260,
    ProcessPerishableCargo,
    ChooseSystemArrivalLocation,
    Size = ChooseSystemArrivalLocation,
    Start = 0x8746C,
    End = 0x876BC
};

enum class ILootInspectVTable
{
    GetPosition,
    GetVelocity,
    GetAngularVelocity,
    GetOrientation,
    GetTransform,
    GetCenterOfMass,
    GetSubObjCenterOfMass,
    GetIndex,
    GetId,
    GetGoodId,
    GetArchetypeExtents,
    GetPhysicalRadius,
    GetMass,
    IsTargetable,
    IsDying,
    GetStatus,
    GetStatus2,
    GetShieldStatus,
    GetShieldStatus2,
    GetThrottle,
    GetAxisThrottle,
    GetNudgeVec,
    GetStrafeDir,
    IsCruiseActive,
    IsCruiseActive2,
    AreThrustersActive,
    GetAttitudeTowards,
    GetAttitudeTowardsSymmetrical,
    GetReputation,
    GetTarget,
    GetSubtarget,
    GetSubtargetCenterOfMass,
    GetRank,
    GetAffiliation,
    GetType,
    GetBase,
    GetDockTarget,
    GetPower,
    GetZoneProps,
    GetScannerInterference,
    GetHoldLeft,
    EnumerateCargo,
    GetData,
    GetFormationOffset,
    GetFormationLeader,
    GetFollowOffset,
    GetFollowOrder,
    IsPlayer,
    GetHardpoint,
    HasDockHardpoints,
    GetDockHardpoints,
    GetTimeToAccelerate,
    GetDistanceTravelled,
    GetProjectedThrottle,
    GetSpeed,
    GetInitialSpeedToCoastDistance,
    GetTimeToAccelerateAngularly,
    GetTimeToAccelerateAngularly2,
    GetAngularDistanceTravelled,
    GetAngularDistanceTravelled2,
    GetAngularSpeedXY,
    GetAngularSpeedZ,
    GetProjectedAxisThrottleXY,
    GetProjectedAxisThrottleZ,
    GetMaxBankAngle,
    GetScanList,
    GetTargetLeadFirePosition,
    IsPointingAt,
    CanPointAt,
    FindEquipment,
    GetEquipmentStatus,
    GetEquipmentVal,
    ScanCargo,
    EnumerateSubtargets,
    GetLaneDirection,
    GetRingSide,
    TraverseRings,
    IsUsingTradelane,
    GetLaneStart,
    GenerateFollowOffsets,
    GetAtmosphereRange,
    GetToughness,
    GetBehaviorId,
    GetFormationFollowers,
    CObject,
    ObjectType,
    ObjectDestroyed,
    Disconnect,
    GetDunno0x40,
    InstantiateCObject,
    SetCObjectById,
    Update,
    sub_6D01A60,
    sub_6CEE810,
    sub_6CEE980,
    sub_6CE9250,
    MunitionImpact,
    ProcessExplosionDamage,
    sub_6D01A10,
    get_dunno_0x41,
    sub_6CEEFA0,
    sub_6CEF0F0,
    IsPlayerVulnerable,
    IsInvulnerable,
    GetMaxHpLoss,
    ProcessCollisionUnk,
    SetHpAndColGrpUnk,
    SetRelativeHealth,
    DamageHull,
    ApplyDamageEntry,
    CanDealDamage,
    Size = CanDealDamage,
    Start = 0x87DF4,
    End = 0x87FAC
};

enum class IGuidedInspectVTable
{
    GetPosition,
    GetVelocity,
    GetAngularVelocity,
    GetOrientation,
    GetTransform,
    GetCenterOfMass,
    GetSubObjCenterOfMass,
    GetIndex,
    GetId,
    GetGoodId,
    GetArchetypeExtents,
    GetPhysicalRadius,
    GetMass,
    IsTargetable,
    IsDying,
    GetStatus,
    GetStatus2,
    GetShieldStatus,
    GetShieldStatus2,
    GetThrottle,
    GetAxisThrottle,
    GetNudgeVec,
    GetStrafeDir,
    IsCruiseActive,
    IsCruiseActive2,
    AreThrustersActive,
    GetAttitudeTowards,
    GetAttitudeTowardsSymmetrical,
    GetReputation,
    GetTarget,
    GetSubtarget,
    GetSubtargetCenterOfMass,
    GetRank,
    GetAffiliation,
    GetType,
    GetBase,
    GetDockTarget,
    GetPower,
    GetZoneProps,
    GetScannerInterference,
    GetHoldLeft,
    EnumerateCargo,
    GetData,
    GetFormationOffset,
    GetFormationLeader,
    GetFollowOffset,
    GetFollowOrder,
    IsPlayer,
    GetHardpoint,
    HasDockHardpoints,
    GetDockHardpoints,
    GetTimeToAccelerate,
    GetDistanceTravelled,
    GetProjectedThrottle,
    GetSpeed,
    GetInitialSpeedToCoastDistance,
    GetTimeToAccelerateAngularly,
    GetTimeToAccelerateAngularly2,
    GetAngularDistanceTravelled,
    GetAngularDistanceTravelled2,
    GetAngularSpeedXY,
    GetAngularSpeedZ,
    GetProjectedAxisThrottleXY,
    GetProjectedAxisThrottleZ,
    GetMaxBankAngle,
    GetScanList,
    GetTargetLeadFirePosition,
    IsPointingAt,
    CanPointAt,
    FindEquipment,
    GetEquipmentStatus,
    GetEquipmentVal,
    ScanCargo,
    EnumerateSubtargets,
    GetLaneDirection,
    GetRingSide,
    TraverseRings,
    IsUsingTradelane,
    GetLaneStart,
    GenerateFollowOffsets,
    GetAtmosphereRange,
    GetToughness,
    GetBehaviorId,
    GetFormationFollowers,
    CObject,
    ObjectType,
    ObjectDestroyed,
    Disconnect,
    GetDunno0x40,
    InstantiateCObject,
    SetCObjectById,
    Update,
    sub_6D01A60,
    sub_6CEE810,
    sub_6CEE980,
    sub_6CE9250,
    MunitionImpact,
    ProcessExplosionDamage,
    sub_6D01A10,
    get_dunno_0x41,
    sub_6CEEFA0,
    sub_6CEF0F0,
    IsPlayerVulnerable,
    IsInvulnerable,
    GetMaxHpLoss,
    ProcessCollisionUnk,
    SetHpAndColGrpUnk,
    SetRelativeHealth,
    DamageHull,
    ApplyDamageEntry,
    CanDealDamage,
    Start = 0x8653C,
    End = 0x866F8
};

enum class IMineInspectVTable
{
    GetPosition,
    GetVelocity,
    GetAngularVelocity,
    GetOrientation,
    GetTransform,
    GetCenterOfMass,
    GetSubObjCenterOfMass,
    GetIndex,
    GetId,
    GetGoodId,
    GetArchetypeExtents,
    GetPhysicalRadius,
    GetMass,
    IsTargetable,
    IsDying,
    GetStatus,
    GetStatus2,
    GetShieldStatus,
    GetShieldStatus2,
    GetThrottle,
    GetAxisThrottle,
    GetNudgeVec,
    GetStrafeDir,
    IsCruiseActive,
    IsCruiseActive2,
    AreThrustersActive,
    GetAttitudeTowards,
    GetAttitudeTowardsSymmetrical,
    GetReputation,
    GetTarget,
    GetSubtarget,
    GetSubtargetCenterOfMass,
    GetRank,
    GetAffiliation,
    GetType,
    GetBase,
    GetDockTarget,
    GetPower,
    GetZoneProps,
    GetScannerInterference,
    GetHoldLeft,
    EnumerateCargo,
    GetData,
    GetFormationOffset,
    GetFormationLeader,
    GetFollowOffset,
    GetFollowOrder,
    IsPlayer,
    GetHardpoint,
    HasDockHardpoints,
    GetDockHardpoints,
    GetTimeToAccelerate,
    GetDistanceTravelled,
    GetProjectedThrottle,
    GetSpeed,
    GetInitialSpeedToCoastDistance,
    GetTimeToAccelerateAngularly,
    GetTimeToAccelerateAngularly2,
    GetAngularDistanceTravelled,
    GetAngularDistanceTravelled2,
    GetAngularSpeedXY,
    GetAngularSpeedZ,
    GetProjectedAxisThrottleXY,
    GetProjectedAxisThrottleZ,
    GetMaxBankAngle,
    GetScanList,
    GetTargetLeadFirePosition,
    IsPointingAt,
    CanPointAt,
    FindEquipment,
    GetEquipmentStatus,
    GetEquipmentVal,
    ScanCargo,
    EnumerateSubtargets,
    GetLaneDirection,
    GetRingSide,
    TraverseRings,
    IsUsingTradelane,
    GetLaneStart,
    GenerateFollowOffsets,
    GetAtmosphereRange,
    GetToughness,
    GetBehaviorId,
    GetFormationFollowers,
    CObject,
    ObjectType,
    ObjectDestroyed,
    Disconnect,
    GetDunno0x40,
    InstantiateCObject,
    SetCObjectById,
    Update,
    sub_6D01A60,
    sub_6CEE810,
    sub_6CEE980,
    sub_6CE9250,
    MunitionImpact,
    ProcessExplosionDamage,
    sub_6D01A10,
    get_dunno_0x41,
    sub_6CEEFA0,
    sub_6CEF0F0,
    IsPlayerVulnerable,
    IsInvulnerable,
    GetMaxHpLoss,
    ProcessCollisionUnk,
    SetHpAndColGrpUnk,
    SetRelativeHealth,
    DamageHull,
    ApplyDamageEntry,
    CanDealDamage,
    Start = 0x8606C,
    End = 0x86228
};