#include "PCH.hpp"

#include "API/Utils/LuaHelper.hpp"

#include "API/FLHook/ResourceManager.hpp"

void LuaHelper::LuaPrint(lua_State* lua, const std::wstring& text, const LogLevel level)
{
    lua_Debug ar;
    lua_getstack(lua, 1, &ar);
    lua_getinfo(lua, "nSl", &ar);

    std::wstring source = std::string_view(ar.short_src).starts_with("[string") ? L"MEMORY" : StringUtils::stows(std::string_view(ar.short_src));
    const auto print = std::format(L"Lua File: {}, Line {} - {}", source, ar.currentline, text);

    if (level == LogLevel::Info)
    {
        INFO(print);
    }
    else if (level == LogLevel::Debug)
    {
        DEBUG(print);
    }
    else if (level == LogLevel::Error)
    {
        ERROR(print);
    }
    else if (level == LogLevel::Warn)
    {
        WARN(print);
    }
    else if (level == LogLevel::Trace)
    {
        TRACE(print);
    }
}

void LuaHelper::InitialiseDefaultLuaState(sol::state* lua)
{
    lua->open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::debug, sol::lib::math, sol::lib::package, sol::lib::utf8);

    sol::usertype<Vector> vectorType = lua->new_usertype<Vector>("Vector", sol::constructors<Vector(), Vector(float, float, float)>());
    vectorType["x"] = &Vector::x;
    vectorType["y"] = &Vector::y;
    vectorType["z"] = &Vector::z;
    vectorType["InRadius"] = &Vector::InRadius;

#define DEFINE_ACTION_TYPE(type)                                                                                                   \
    {                                                                                                                              \
        sol::usertype<Action<type>> PPCAT(actionType_, LINE) = lua->new_usertype<Action<type>>(PPCAT("action_", STRINGIZE(type))); \
        PPCAT(actionType_, LINE)["Handle"] = &Action<type>::Handle;                                                                \
        PPCAT(actionType_, LINE)["Unwrap"] = &Action<type>::Unwrap;                                                                \
        PPCAT(actionType_, LINE)["HasError"] = &Action<type>::HasError;                                                            \
        PPCAT(actionType_, LINE)["Error"] = &Action<type>::Error;                                                                  \
        PPCAT(actionType_, LINE)["HasValue"] = &Action<type>::HasValue;                                                            \
        PPCAT(actionType_, LINE)["Value"] = &Action<type>::Value;                                                                  \
    }

    DEFINE_ACTION_TYPE(std::string);
    DEFINE_ACTION_TYPE(std::wstring);
    DEFINE_ACTION_TYPE(std::wstring_view);
    DEFINE_ACTION_TYPE(uint);
    DEFINE_ACTION_TYPE(int);
    DEFINE_ACTION_TYPE(float);
    DEFINE_ACTION_TYPE(SystemId);
    DEFINE_ACTION_TYPE(BaseId);
    DEFINE_ACTION_TYPE(RepGroupId);
    DEFINE_ACTION_TYPE(RepId);
    DEFINE_ACTION_TYPE(ObjectId);
    DEFINE_ACTION_TYPE(ShipId);
    DEFINE_ACTION_TYPE(ClientId);
    DEFINE_ACTION_TYPE(GoodId);
    DEFINE_ACTION_TYPE(GroupId);
    DEFINE_ACTION_TYPE(EquipmentId);
    DEFINE_ACTION_TYPE(std::vector<ClientId>);
    DEFINE_ACTION_TYPE(std::vector<BaseId>);
    DEFINE_ACTION_TYPE(std::vector<SystemId>);
    DEFINE_ACTION_TYPE(std::vector<ObjectId>);
    DEFINE_ACTION_TYPE(std::vector<ShipId>);
    DEFINE_ACTION_TYPE(std::vector<GoodId>);
    DEFINE_ACTION_TYPE(std::vector<EquipmentId>);
    DEFINE_ACTION_TYPE(std::vector<RepGroupId>);
    DEFINE_ACTION_TYPE(std::vector<RepId>);
    DEFINE_ACTION_TYPE(Vector);

    // Handle void type
    {
        sol::usertype<Action<void>> voidType = lua->new_usertype<Action<void>>("action_void");
        voidType["Handle"] = &Action<void>::Handle;
        voidType["HasError"] = &Action<void>::HasError;
        voidType["Error"] = &Action<void>::Error;
    }

#undef DEFINE_ACTION_TYPE

    lua->set_function("create_id", [](const std::string& id) { return CreateID(id.c_str()); });
    lua->set_function("make_id", [](const std::string& id) { return MakeId(id.c_str()); });

#define ClsFunc(func) PPCAT(lua_, TYPE)[STRINGIZE(func)] = &TYPE::##func
#define NEW_TYPE(...) sol::usertype<TYPE> PPCAT(lua_, TYPE) = lua -> new_usertype<TYPE>(STRINGIZE(TYPE), sol::constructors<__VA_ARGS__>())

#define TYPE          SystemId
    NEW_TYPE(TYPE(), TYPE(uint));
    ClsFunc(GetValue);
    ClsFunc(GetName);
    ClsFunc(GetNickName);
    // ClsFunc(GetZones); TODO: Add lua IZone support
    ClsFunc(PositionToSectorCoord);
    ClsFunc(GetNeighboringSystems);
    // ClsFunc(GetSolars); TODO: Add lua CSolar or maybe some kind of SolarId support
    ClsFunc(GetPlayersInSystem);
    ClsFunc(Message);
    ClsFunc(PlaySoundOrMusic);
    ClsFunc(KillAllPlayers);
#undef TYPE
#define TYPE ClientId
    NEW_TYPE(TYPE(), TYPE(uint), TYPE(std::wstring));
    ClsFunc(IsValidClientId);
    ClsFunc(GetValue);
    ClsFunc(GetCharacterName);
    ClsFunc(GetCurrentBase);
    ClsFunc(GetSystemId);
    ClsFunc(GetShipArch);
    ClsFunc(GetShip);
    ClsFunc(GetLatency);
    ClsFunc(GetGroup);
    ClsFunc(GetReputation);
    ClsFunc(GetRank);
    ClsFunc(GetPosition);
    ClsFunc(GetRelativeHealth);
    ClsFunc(SetRelativeHealth);
    // ClsFunc(GetEquipCargo); // TODO: Implement Lua st6 list EquipDesc
    ClsFunc(GetRemainingCargo);
    // ClsFunc(GetEquipCargo); TODO: Implement Lua CollisionGroupDesc & st6 list
    // ClsFunc(GetData); TODO: Implement ClientData
    ClsFunc(GetPlayerIp);
    ClsFunc(GetEngineState);
    ClsFunc(InSpace);
    ClsFunc(IsDocked);
    ClsFunc(InCharacterSelect);
    ClsFunc(IsAlive);
    ClsFunc(Kick);
    ClsFunc(SaveChar);
    ClsFunc(SetPvpKills);
    ClsFunc(SetCash);
    ClsFunc(AddCash);
    ClsFunc(RemoveCash);
    ClsFunc(Beam);
    // ClsFunc(Rename); TODO Implement Lua rename
    ClsFunc(MarkObject);
    ClsFunc(Message);
    ClsFunc(MessageErr);
    ClsFunc(MessageLocal);
    ClsFunc(MessageFrom);
    ClsFunc(MessageCustomXml);
    ClsFunc(SetEquip);
    ClsFunc(AddEquip);
    ClsFunc(AddCargo);
    ClsFunc(RemoveCargo);
    ClsFunc(Undock);
    ClsFunc(PlaySound);
    ClsFunc(InvitePlayer);
    ClsFunc(SendBestPath);
#undef TYPE
#define TYPE BaseId
    NEW_TYPE(TYPE(), TYPE(uint), TYPE(std::wstring_view, bool));
    ClsFunc(GetValue);
    ClsFunc(GetSpaceId);
    ClsFunc(GetSystem);
    ClsFunc(GetAffiliation);
    ClsFunc(GetName);
    ClsFunc(GetBaseHealth);
    ClsFunc(GetDescription);
    ClsFunc(GetItemsForSale);
    ClsFunc(GetCommodityPrice);
    ClsFunc(GetDockedPlayers);
#undef TYPE
#define TYPE ObjectId
    NEW_TYPE(TYPE(), TYPE(uint));
    ClsFunc(GetValue);
    ClsFunc(GetId);
    ClsFunc(GetObjectType);
    ClsFunc(GetNickName);
    ClsFunc(GetArchetype);
    ClsFunc(GetVelocityAndSpeed);
    ClsFunc(GetAngularVelocity);
    ClsFunc(GetPositionAndOrientation);
    ClsFunc(GetSystem);
    ClsFunc(GetReputation);
    ClsFunc(GetHealth);
    ClsFunc(GetPlayer);
#undef TYPE
#define TYPE RepId
    NEW_TYPE(TYPE(), TYPE(uint));
    ClsFunc(GetAffiliation);
    ClsFunc(GetValue);
    ClsFunc(GetAttitudeTowardsRepId);
    ClsFunc(GetAttitudeTowardsFaction);
    ClsFunc(GetRank);
    ClsFunc(SetRank);
    ClsFunc(SetAttitudeTowardsRepId);
    ClsFunc(SetAttitudeTowardsRepGroupId);
    ClsFunc(SetAffiliation);
    // ClsFunc(GetName); TODO: Implement Lua FmtStr
#undef TYPE
#define TYPE RepGroupId
    NEW_TYPE(TYPE(), TYPE(uint), TYPE(std::wstring_view));

    ClsFunc(GetValue);
    ClsFunc(GetName);
    ClsFunc(GetShortName);
    ClsFunc(GetAttitudeTowardsRepId);
#undef TYPE
#define TYPE EquipmentId
    NEW_TYPE(TYPE(), TYPE(uint));

    ClsFunc(GetValue);
    ClsFunc(GetId);
    ClsFunc(GetType);
    ClsFunc(GetName);
    ClsFunc(GetVolume);
#undef TYPE
#define TYPE ShipId
    NEW_TYPE(TYPE(), TYPE(uint));

    ClsFunc(GetShipArchetype);
    ClsFunc(GetShields);
    ClsFunc(GetPlayer);
    ClsFunc(GetTarget);
    ClsFunc(GetReputation);
    ClsFunc(GetSpeed);
    ClsFunc(IsPlayer);
    ClsFunc(IsNpc);
    ClsFunc(IsInTradeLane);
    ClsFunc(Destroy);
    ClsFunc(SetHealth);
    ClsFunc(AddCargo);
    ClsFunc(Relocate);
    ClsFunc(IgniteFuse);
    ClsFunc(ExtinguishFuse);
    ClsFunc(GetEquipmentManager);
#undef TYPE
#define TYPE GroupId
    NEW_TYPE(TYPE(), TYPE(uint));
    ClsFunc(GetValue);
    ClsFunc(GetGroupMembers);
    ClsFunc(GetGroupSize);
    ClsFunc(ForEachGroupMember);
    ClsFunc(InviteMember);
    ClsFunc(AddMember);
#undef TYPE
#undef ClsFunc
#undef NEW_TYPE

    auto state = lua->lua_state();
    auto logTable = lua->create_named_table("log");
    logTable.set_function("info", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Info); });
    logTable.set_function("debug", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Debug); });
    logTable.set_function("error", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Error); });
    logTable.set_function("warn", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Warn); });
    logTable.set_function("trace", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Trace); });

    auto builder =
        lua->new_usertype<ResourceManager::SpaceObjectBuilder>("SpaceObjectBuilder", sol::factories([] { return FLHook::GetResourceManager()->NewBuilder(); }));
    builder["WithNpc"] = &ResourceManager::SpaceObjectBuilder::WithNpc;
    builder["WithArchetype"] =
        static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(uint)>(&ResourceManager::SpaceObjectBuilder::WithArchetype);
    builder["WithLoadout"] =
        static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(uint)>(&ResourceManager::SpaceObjectBuilder::WithLoadout);
    builder["WithPersonality"] = static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(const std::wstring&)>(
        &ResourceManager::SpaceObjectBuilder::WithPersonality);
    builder["WithStateGraph"] = &ResourceManager::SpaceObjectBuilder::WithStateGraph;
    builder["WithPosition"] = &ResourceManager::SpaceObjectBuilder::WithPosition;
    builder["WithRotation"] = static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(const Vector&)>(
        &ResourceManager::SpaceObjectBuilder::WithRotation);
    builder["WithSystem"] =
        static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(SystemId)>(&ResourceManager::SpaceObjectBuilder::WithSystem);
    builder["WithAbsoluteHealth"] = &ResourceManager::SpaceObjectBuilder::WithAbsoluteHealth;
    builder["WithRelativeHealth"] = &ResourceManager::SpaceObjectBuilder::WithRelativeHealth;
    builder["WithLevel"] = &ResourceManager::SpaceObjectBuilder::WithLevel;
    builder["WithVoice"] = &ResourceManager::SpaceObjectBuilder::WithVoice;
    builder["WithName"] =
        static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(uint, uint)>(&ResourceManager::SpaceObjectBuilder::WithName);
    builder["WithReputation"] = static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(RepGroupId)>(
        &ResourceManager::SpaceObjectBuilder::WithReputation);
    builder["WithDockTo"] = &ResourceManager::SpaceObjectBuilder::WithDockTo;
    builder["WithFuse"] = &ResourceManager::SpaceObjectBuilder::WithFuse;
    builder["WithRandomNpc"] = &ResourceManager::SpaceObjectBuilder::WithRandomNpc;
    builder["WithRandomReputation"] = &ResourceManager::SpaceObjectBuilder::WithRandomReputation;
    builder["WithRandomName"] = &ResourceManager::SpaceObjectBuilder::WithRandomName;
    builder["AsSolar"] = &ResourceManager::SpaceObjectBuilder::AsSolar;
    builder["AsNpc"] = &ResourceManager::SpaceObjectBuilder::AsNpc;
    builder["Spawn"] = &ResourceManager::SpaceObjectBuilder::Spawn;
}
