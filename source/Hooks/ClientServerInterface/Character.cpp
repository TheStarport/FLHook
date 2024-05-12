#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "API/Utils/PerfTimer.hpp"

#include "API/Utils/Logger.hpp"
#include "Core/ClientServerInterface.hpp"

bool IServerImplHook::CharacterSelectInner(const CHARACTER_ID& cid, const ClientId client)
{
    try
    {
        auto& info = client.GetData();
        const auto charName = client.GetCharacterName().Unwrap();
        charBefore = !charName.empty() ? charName : L"";
        info.lastExitedBaseId = 0;
        info.tradePartner = ClientId();
        info.groupId = Players.GetGroupID(client.GetValue());
    }
    catch (...)
    {
        // AddKickLog(client, "Corrupt character file?");
        client.Kick();
        return false;
    }

    return true;
}

void IServerImplHook::CharacterSelectInnerAfter([[maybe_unused]] const CHARACTER_ID& charId, ClientId client)
{
    TryHook
    {
        auto& info = client.GetData();

        if (const auto charName = client.GetCharacterName().Handle(); charBefore != charName)
        {
            CallPlugins(&Plugin::OnLoadCharacterSettings, client, std::wstring_view(charName));

            if (FLHook::GetConfig().userCommands.userCmdHelp)
            {
                (void)client.Message(L"To get a list of available commands, type \"/help\" in chat.");
            }

            auto cargoList = client.GetEquipCargo().Raw();
            if (cargoList.has_error())
            {
                (void)client.Kick();
                return;
            }

            for (const auto& cargo : *cargoList.value())
            {
                if (cargo.count < 0)
                {
                    // AddCheaterLog(charName, "Negative good-count, likely to have cheated in the past");

                    FLHook::MessageUniverse(std::format(L"Possible cheating detected: {}", charName));
                    (void)client.Kick();
                    (void)client.GetAccount().Unwrap().Ban();
                    return;
                }
            }

            if (FLHook::GetConfig().general.persistGroup && info.groupId)
            {
                GroupId(info.groupId).AddMember(client);
            }
            else
            {
                info.groupId = 0;
            }

            // Assign their random formation id.
            // Numbers are between 0-20 (inclusive)
            // Formations are between 1-29 (inclusive)
            static std::random_device dev;
            static std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> distNum(1, 20);
            const auto& config = FLHook::GetConfig();
            std::uniform_int_distribution<std::mt19937::result_type> distForm(0, config.callsign.allowedFormations.size() - 1);

            if (config.callsign.allowedFormations.empty())
            {
                info.formationTag = AllowedFormation::Alpha;
            }
            else
            {
                info.formationTag = config.callsign.allowedFormations[distForm(rng)];
            }

            info.formationNumber1 = distNum(rng);
            info.formationNumber2 = distNum(rng);
        }
    }
    CatchHook({})
}

void __stdcall IServerImplHook::CharacterSelect(const CHARACTER_ID& cid, ClientId client)
{
    Logger::Trace(std::format(L"CharacterSelect(\n\tClientId client = {}\n)", client));

    auto& data = AccountManager::accounts[client.GetValue()].characters.at(cid.charFilename);
    FLHook::GetClient(client).characterName = data.wideCharacterName;

    std::wstring charName = StringUtils::stows(static_cast<const char*>(cid.charFilename));
    const auto skip = CallPlugins(&Plugin::OnCharacterSelect, client);

    CheckForDisconnect;

    if (const bool innerCheck = CharacterSelectInner(cid, client); !innerCheck)
    {
        return;
    }
    if (!skip)
    {
        CallServerPreamble { Server.CharacterSelect(cid, client.GetValue()); }
        CallServerPostamble(true, );
    }
    CharacterSelectInnerAfter(cid, client);

    CallPlugins(&Plugin::OnCharacterSelectAfter, client);
}

void __stdcall IServerImplHook::CreateNewCharacter(const SCreateCharacterInfo& createCharacterInfo, ClientId client)
{
    Logger::Trace(std::format(L"CreateNewCharacter(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnCharacterCreation, client, createCharacterInfo); !skip)
    {
        TaskScheduler::Schedule(std::bind(AccountManager::OnCreateNewCharacterCopy, &Players[client.GetValue()], createCharacterInfo));
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
    Logger::Trace(std::format(L"DestroyCharacter(\n\tClientId client = {}\n)", client));

    const std::wstring charName = StringUtils::stows(static_cast<const char*>(cid.charFilename));

    if (const auto skip = CallPlugins(&Plugin::OnCharacterDelete, client, std::wstring_view(charName)); !skip)
    {
        CHARACTER_ID cidCopy = cid;
        TaskScheduler::ScheduleWithCallback(std::bind(AccountManager::DeleteCharacter, client, charName),
            std::bind(IServerImplHook::DestroyCharacterCallback, client, cidCopy));
    }
}

void __stdcall IServerImplHook::RequestRankLevel(ClientId client, uint unk1, int unk2)
{
    Logger::Trace(std::format(L"RequestRankLevel(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnRequestRankLevel, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestRankLevel(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestRankLevelAfter, client, unk1, unk2);
}

void __stdcall IServerImplHook::RequestPlayerStats(ClientId client, uint unk1, int unk2)
{
    Logger::Trace(std::format(L"RequestPlayerStats(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

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
        auto& info = client.GetData();
        if (info.charMenuEnterTime)
        {
            // pushed f1
            uint shipId = 0;
            pub::Player::GetShip(client.GetValue(), shipId);
            if (shipId)
            {
                // in space
                info.f1Time = TimeUtils::UnixTime<std::chrono::milliseconds>() + FLHook::GetConfig().autoKicks.antiF1;
                return false;
            }
        }
    }
    CatchHook({});

    return true;
}

bool CharacterInfoReqCatch(ClientId client, bool)
{
    // AddKickLog(client, "Corrupt charfile?");
    client.Kick();
    return false;
}

void __stdcall IServerImplHook::CharacterInfoReq(ClientId client, bool unk1)
{
    Logger::Trace(std::format(L"CharacterInfoReq(\n\tClientId client = {}\n\tbool unk1 = {}\n)", client, unk1));

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
