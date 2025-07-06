#include "PCH.hpp"

#include "API/Utils/LuaHelper.hpp"

#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/Utils/PathHelper.hpp"

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

    sol::usertype<Matrix> matrixType = lua->new_usertype<Matrix>("Matrix");
    matrixType["ToEuler"] = &Matrix::ToEuler;
    matrixType["FromEuler"] = &Matrix::FromEuler;
    matrixType["Identity"] = &Matrix::Identity;

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
    DEFINE_ACTION_TYPE(Matrix);

    // Handle void type
    {
        sol::usertype<Action<void>> voidType = lua->new_usertype<Action<void>>("action_void");
        voidType["Handle"] = &Action<void>::Handle;
        voidType["HasError"] = &Action<void>::HasError;
        voidType["Error"] = &Action<void>::Error;
    }

    // Enums
    auto errors = lua->create_named_table("Error");
    for (const auto& [val, name] : magic_enum::enum_entries<Error>())
    {
        errors[name] = val;
    }

    auto engineState = lua->create_named_table("EngineState");
    for (const auto& [val, name] : magic_enum::enum_entries<EngineState>())
    {
        errors[name] = val;
    }

    auto msgColor = lua->create_named_table("MessageColor");
    for (const auto& [val, name] : magic_enum::enum_entries<MessageColor>())
    {
        msgColor[name] = val;
    }

    auto msgFormat = lua->create_named_table("MessageFormat");
    for (const auto& [val, name] : magic_enum::enum_entries<MessageFormat>())
    {
        msgFormat[name] = val;
    }

    auto objectClass = lua->create_named_table("ObjectClass");
    for (const auto& [val, name] : magic_enum::enum_entries<CObject::Class>())
    {
        objectClass[name] = val;
    }

    auto equipmentClass = lua->create_named_table("EquipmentType");
    for (const auto& [val, name] : magic_enum::enum_entries<EquipmentType>())
    {
        objectClass[name] = val;
    }

#undef DEFINE_ACTION_TYPE

    lua->set_function("create_id", [](const std::string& id) { return CreateID(id.c_str()); });
    lua->set_function("make_id", [](const std::string& id) { return MakeId(id.c_str()); });

#define ClsFunc(func) PPCAT(lua_, TYPE)[STRINGIZE(func)] = &TYPE::##func
#define NEW_TYPE(...) \
    sol::usertype<TYPE> PPCAT(lua_, TYPE) = lua -> new_usertype<TYPE>(STRINGIZE(TYPE), sol::constructors<__VA_ARGS__>()) // NOLINT(*-macro-usage)

#define TYPE SystemId
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
    // ClsFunc(MarkObject);
    ClsFunc(Message);
    ClsFunc(MessageErr);
    ClsFunc(MessageLocal);
    ClsFunc(MessageFrom);
    ClsFunc(MessageCustomXml);
    // ClsFunc(SetEquip);
    ClsFunc(AddEquip);
    ClsFunc(AddCargo);
    ClsFunc(RemoveCargo);
    ClsFunc(Undock);
    ClsFunc(PlaySound);
    ClsFunc(DisplayMissionObjective);
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
    // ClsFunc(GetValue);
    ClsFunc(GetId);
    ClsFunc(GetNickName);
    // ClsFunc(GetArchetype);
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

    // ClsFunc(GetValue);
    ClsFunc(GetId);
    ClsFunc(GetType);
    ClsFunc(GetName);
    ClsFunc(GetVolume);
#undef TYPE
#define TYPE ShipId
    NEW_TYPE(TYPE(), TYPE(uint));

    // ClsFunc(GetShipArchetype); TODO: Implement ship archetype lua interface
    // ClsFunc(GetValue);
    ClsFunc(GetId);
    ClsFunc(GetNickName);
    // ClsFunc(GetArchetype);
    ClsFunc(GetVelocityAndSpeed);
    ClsFunc(GetAngularVelocity);
    ClsFunc(GetPositionAndOrientation);
    ClsFunc(GetSystem);
    ClsFunc(GetReputation);
    ClsFunc(GetHealth);
    ClsFunc(GetPlayer);
    // Ship methods
    ClsFunc(GetShields);
    ClsFunc(GetTarget);
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
    // ClsFunc(GetEquipmentManager); TODO: Implement CEquipmentManager lua interface
#undef TYPE
#define TYPE GroupId
    NEW_TYPE(TYPE(), TYPE(uint));
    ClsFunc(GetValue);
    ClsFunc(GetGroupMembers);
    ClsFunc(GetGroupSize);
    // ClsFunc(ForEachGroupMember);
    ClsFunc(InviteMember);
    ClsFunc(AddMember);
#undef TYPE
#undef ClsFunc
#undef NEW_TYPE

    auto* state = lua->lua_state();
    auto logTable = lua->create_named_table("log");
    logTable.set_function("info", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Info); });
    logTable.set_function("debug", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Debug); });
    logTable.set_function("error", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Error); });
    logTable.set_function("warn", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Warn); });
    logTable.set_function("trace", [state](const std::wstring& str) { LuaPrint(state, str, LogLevel::Trace); });

    auto pathEntry = lua->new_usertype<PathEntry>("PathEntry", sol::constructors<PathEntry()>());
    pathEntry["pos"] = &PathEntry::pos;
    pathEntry["obj_id"] = &PathEntry::objId;
    pathEntry["system_id"] = &PathEntry::systemId;

    auto pathHelper = lua->create_named_table("path_helper");
    pathHelper.set_function("clear_waypoints", PathHelper::ClearWaypoints);
    pathHelper.set_function("create_object_waypoints", PathHelper::CreateObjectWaypoint);
    pathHelper.set_function("create_clearable_waypoints", PathHelper::CreateClearableWaypoints);

    // TODO: Uncomment once fixed downstream https://github.com/ThePhD/sol2/pull/1676
    // auto infocardPayload = lua->new_usertype<InfocardManager::InfocardPayload>("InfocardPayload", sol::constructors<InfocardManager::InfocardPayload()>());
    // infocardPayload["info_cards"] = &InfocardManager::InfocardPayload::infoCards;
    // infocardPayload["info_names"] = &InfocardManager::InfocardPayload::infoNames;

    auto infocardManager = lua->new_usertype<InfocardManager>("InfocardManager", sol::factories([] { return FLHook::GetInfocardManager(); }));
    infocardManager["get_info_name"] = &InfocardManager::GetInfoName;
    infocardManager["override_infocard"] = &InfocardManager::OverrideInfocard;
    // infocardManager["override_infocards"] = &InfocardManager::OverrideInfocards;
    infocardManager["clear_override"] = &InfocardManager::ClearOverride;

    auto builder =
        lua->new_usertype<ResourceManager::SpaceObjectBuilder>("SpaceObjectBuilder", sol::factories([] { return FLHook::GetResourceManager()->NewBuilder(); }));
    builder["with_npc"] = &ResourceManager::SpaceObjectBuilder::WithNpc;
    builder["with_archetype"] =
        static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(Id)>(&ResourceManager::SpaceObjectBuilder::WithArchetype);
    builder["with_loadout"] =
        static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(uint)>(&ResourceManager::SpaceObjectBuilder::WithLoadout);
    builder["with_personality"] = static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(const std::wstring&)>(
        &ResourceManager::SpaceObjectBuilder::WithPersonality);
    builder["with_state_graph"] = &ResourceManager::SpaceObjectBuilder::WithStateGraph;
    builder["with_position"] = &ResourceManager::SpaceObjectBuilder::WithPosition;
    builder["with_rotation"] = static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(const Vector&)>(
        &ResourceManager::SpaceObjectBuilder::WithRotation);
    builder["with_system"] =
        static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(SystemId)>(&ResourceManager::SpaceObjectBuilder::WithSystem);
    builder["with_absolute_health"] = &ResourceManager::SpaceObjectBuilder::WithAbsoluteHealth;
    builder["with_relative_health"] = &ResourceManager::SpaceObjectBuilder::WithRelativeHealth;
    builder["with_level"] = &ResourceManager::SpaceObjectBuilder::WithLevel;
    builder["with_voice"] = &ResourceManager::SpaceObjectBuilder::WithVoice;
    builder["with_name"] =
        static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(uint, uint)>(&ResourceManager::SpaceObjectBuilder::WithName);
    builder["with_reputation"] = static_cast<ResourceManager::SpaceObjectBuilder& (ResourceManager::SpaceObjectBuilder::*)(RepGroupId)>(
        &ResourceManager::SpaceObjectBuilder::WithReputation);
    builder["with_dock_to"] = &ResourceManager::SpaceObjectBuilder::WithDockTo;
    builder["with_fuse"] = &ResourceManager::SpaceObjectBuilder::WithFuse;
    builder["with_random_npc"] = &ResourceManager::SpaceObjectBuilder::WithRandomNpc;
    builder["with_random_reputation"] = &ResourceManager::SpaceObjectBuilder::WithRandomReputation;
    builder["with_random_name"] = &ResourceManager::SpaceObjectBuilder::WithRandomName;
    builder["as_solar"] = &ResourceManager::SpaceObjectBuilder::AsSolar;
    builder["as_npc"] = &ResourceManager::SpaceObjectBuilder::AsNpc;
    builder["spawn"] = &ResourceManager::SpaceObjectBuilder::Spawn;
}
