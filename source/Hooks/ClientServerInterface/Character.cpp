#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "API/Utils/PerfTimer.hpp"

#include "API/Utils/Logger.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/IEngineHook.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Core/ExceptionHandler.hpp"
#include "Core/PluginManager.hpp"
#include "Exceptions/StopProcessingException.hpp"

void IServerImplHook::CharacterSelectInnerAfter([[maybe_unused]] const CHARACTER_ID& charId, ClientId client)
{
    TryHook
    {
        IEngineHook::OnCharacterSelectAfter(client);
        auto& info = client.GetData();

        // If help command is not disabled, alert people they can use it
        if (const auto& cmds = FLHook::GetConfig()->userCommands.disabledCommands; std::ranges::find(cmds, L"help") == cmds.end())
        {
            auto& credentials = FLHook::instance->credentialsMap[client];
            (void)client.Message(L"To get a list of available commands, type '/help [page]' in chat.");
            if (!credentials.empty())
            {
                client.Message(L"To get a list of available admin commands, type '.help [page]' in chat.");
            }
        }

        auto cargoList = client.GetEquipCargo().Raw();
        if (cargoList.has_error())
        {
            (void)client.Kick();
            return;
        }

        for (const auto& cargo : cargoList.value()->equip)
        {
            if (cargo.count < 0)
            {
                // AddCheaterLog(charName, "Negative good-count, likely to have cheated in the past");

                FLHook::MessageUniverse(std::format(L"Possible cheating detected: {}", client.GetCharacterId().Handle()));
                (void)client.Kick();
                (void)client.GetAccount().Unwrap().Ban();
                return;
            }
        }

        // Assign their random formation id.
        // Numbers are between 0-20 (inclusive)
        // Formations are between 1-29 (inclusive)
        static std::random_device dev;
        static std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> distNum(1, 20);
        const auto config = FLHook::GetConfig();
        std::uniform_int_distribution<std::mt19937::result_type> distForm(0, config->callsign.allowedFormations.size() - 1);

        if (config->callsign.allowedFormations.empty())
        {
            info.formationTag = AllowedFormation::Alpha;
        }
        else
        {
            info.formationTag = config->callsign.allowedFormations[distForm(rng)];
        }

        info.formationNumber1 = distNum(rng);
        info.formationNumber2 = distNum(rng);
    }
    CatchHook({})
}

void __stdcall IServerImplHook::CharacterSelect(const CHARACTER_ID& cid, ClientId client)
{
    TRACE("IServerImplHook::CharacterSelect client={{client}}", { "client", client });

    auto prevCharName = FLHook::GetClient(client).characterId;

    const auto& data = AccountManager::accounts[client.GetValue()].characters.at(cid.charFilename);
    // TODO what if the client tries to pick a character that isn't on this account
    FLHook::GetClient(client).characterId = CharacterId{ data.wideCharacterName };

    auto charName = CharacterId{ StringUtils::stows(static_cast<const char*>(cid.charFilename)) };
    bool skip = false;

    auto& info = client.GetData();
    if (charName != prevCharName)
    {
        Server.AbortMission(client.GetValue(), 0);
        info.playerData->missionId = 0;
        info.playerData->missionSetBy = 0;

        auto groupId = Players.GetGroupID(client.GetValue());
        CallPlugins(&Plugin::OnClearClientInfo, client);
        skip = CallPlugins(&Plugin::OnCharacterSelect, client);
        if (FLHook::GetConfig()->general.persistGroup)
        {
            GroupId(groupId).AddMember(client);
        }
    }

    info.lastExitedBaseId = {};
    info.tradePartner = {};

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.CharacterSelect(cid, client.GetValue()); }
        CallServerPostamble(true, );
    }

    if (charName != prevCharName)
    {
        CharacterSelectInnerAfter(cid, client);
        CallPlugins(&Plugin::OnCharacterSelectAfter, client);
    }
}

void __stdcall IServerImplHook::CreateNewCharacter(const SCreateCharacterInfo& createCharacterInfo, ClientId client)
{
    TRACE("IServerImplHook::CreateNewCharacter client={{client}}", { "client", client });

    // Ban any name that is numeric and might interfere with commands
    if (const auto numeric = StringUtils::Cast<uint>(std::wstring_view(createCharacterInfo.charname, wcslen(createCharacterInfo.charname)));
        numeric < 10000 && numeric != 0)
    {
        DEBUG("Character name with only numerical values provided. Suppressing character creation.");
        return;
    }

    if (const auto skip = CallPlugins(&Plugin::OnCharacterCreation, client, createCharacterInfo); !skip)
    {
        FLHook::GetTaskScheduler()->ScheduleTask(AccountManager::OnCreateNewCharacterCopy, &Players[client.GetValue()], createCharacterInfo);
    }
}

// ReSharper disable twice CppPassValueParameterByConstReference
void IServerImplHook::DestroyCharacterCallback(const ClientId client, CHARACTER_ID cid)
{
    CallServerPreamble { Server.DestroyCharacter(cid, client.GetValue()); }
    CallServerPostamble(true, );
}

void __stdcall IServerImplHook::DestroyCharacter(const CHARACTER_ID& cid, ClientId client)
{
    TRACE("IServerImplHook::DestroyCharacter client={{client}}", { "client", client });

    const std::wstring charName = StringUtils::stows(static_cast<const char*>(cid.charFilename));

    if (const auto skip = CallPlugins(&Plugin::OnCharacterDelete, client, std::wstring_view(charName)); !skip)
    {
        CHARACTER_ID cidCopy = cid;
        FLHook::GetTaskScheduler()->ScheduleTask(AccountManager::DeleteCharacter, client, charName, cidCopy);
    }
}

void __stdcall IServerImplHook::RequestRankLevel(ClientId client, uint unk1, int unk2)
{
    if (const auto skip = CallPlugins(&Plugin::OnRequestRankLevel, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestRankLevel(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestRankLevelAfter, client, unk1, unk2);
}

void __stdcall IServerImplHook::RequestPlayerStats(ClientId client, uint unk1, int unk2)
{
    if (const auto skip = CallPlugins(&Plugin::OnRequestPlayerStats, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestPlayerStats(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestPlayerStats, client, unk1, unk2);
}

bool IServerImplHook::CharacterInfoReqInner(ClientId client, bool unk1)
{
    TryHook
    {
        if (Players[client.GetValue()].systemId)
        {
            pub::Save(client.GetValue(), 1);
        }

        auto& info = client.GetData();
        if (info.charMenuEnterTime)
        {
            // pushed f1
            uint shipId = 0;
            pub::Player::GetShip(client.GetValue(), shipId);
            if (shipId)
            {
                // in space
                info.f1Time = TimeUtils::UnixTime<std::chrono::milliseconds>() + FLHook::GetConfig()->autoKicks.antiF1;
                return false;
            }
        }
    }
    CatchHook({});

    return true;
}

bool CharacterInfoReqCatch(ClientId client, bool)
{
    (void)client.Kick();
    return false;
}

void __stdcall IServerImplHook::CharacterInfoReq(ClientId client, bool unk1)
{
    TRACE("IServerImplHook::CharacterInfoReq client={{client}} unk1={{unk1}}", { "client", client }, { "unk1", unk1 });

    const auto skip = CallPlugins(&Plugin::OnCharacterInfoRequest, client, unk1);

    CheckForDisconnect;

    if (const bool innerCheck = CharacterInfoReqInner(client, unk1); !innerCheck)
    {
        return;
    }

    if (!skip)
    {
        CallServerPreamble { Server.CharacterInfoReq(client.GetValue(), unk1); }
        CallServerPostamble(CharacterInfoReqCatch(client, unk1), );
    }

    CallPlugins(&Plugin::OnCharacterInfoRequestAfter, client, unk1);
}
