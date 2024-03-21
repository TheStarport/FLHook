#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/ClientList.hpp"
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
        info.characterName = charName;
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

            int hold;
            auto cargoList = client.EnumCargo(hold).Raw();
            if (cargoList.has_error())
            {
                (void)client.Kick();
                return;
            }

            for (const auto& cargo : cargoList.value())
            {
                if (cargo.count < 0)
                {
                    // AddCheaterLog(charName, "Negative good-count, likely to have cheated in the past");

                    FLHook::MessageUniverse(std::format(L"Possible cheating detected: {}", charName));
                    client.Kick();
                    client.GetAccount().Unwrap().Ban();
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

            info.characterName = charName;

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
    Logger::Log(LogLevel::Trace, std::format(L"CharacterSelect(\n\tClientId client = {}\n)", client));

    std::wstring charName = StringUtils::stows(static_cast<const char*>(cid.charFilename));
    const auto skip = CallPlugins(&Plugin::OnCharacterSelect, client, std::wstring_view(charName));

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

    CallPlugins(&Plugin::OnCharacterSelectAfter, client, std::wstring_view(charName));
}

void __stdcall IServerImplHook::CreateNewCharacter(const SCreateCharacterInfo& unk1, ClientId client)
{
    Logger::Log(LogLevel::Trace, std::format(L"CreateNewCharacter(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnCharacterCreation, client, unk1); !skip)
    {
        CallServerPreamble { Server.CreateNewCharacter(unk1, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnCharacterCreationAfter, client, unk1);
}

void __stdcall IServerImplHook::DestroyCharacter(const CHARACTER_ID& cid, ClientId client)
{
    Logger::Log(LogLevel::Trace, std::format(L"DestroyCharacter(\n\tClientId client = {}\n)", client));

    const std::wstring charName = StringUtils::stows(static_cast<const char*>(cid.charFilename));

    if (const auto skip = CallPlugins(&Plugin::OnCharacterDelete, client, std::wstring_view(charName)); !skip)
    {
        FLHook::GetAccountManager().DeleteCharacter(charName);

        CallServerPreamble { Server.DestroyCharacter(cid, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnCharacterDeleteAfter, client, std::wstring_view(charName));
}

void __stdcall IServerImplHook::RequestRankLevel(ClientId client, uint unk1, int unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"RequestRankLevel(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnRequestRankLevel, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestRankLevel(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestRankLevelAfter, client, unk1, unk2);
}

void __stdcall IServerImplHook::RequestPlayerStats(ClientId client, uint unk1, int unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"RequestPlayerStats(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnRequestPlayerStats, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestPlayerStats(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestPlayerStats, client, unk1, unk2);
}

bool IServerImplHook::CharacterInfoReqInner(ClientId client, bool)
{
    TryHook
    {
        auto& info = client.GetData();
        if (!info.charMenuEnterTime)
        {
            info.characterName = client.GetCharacterName().Unwrap();
        }
        else
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
    Logger::Log(LogLevel::Trace, std::format(L"CharacterInfoReq(\n\tClientId client = {}\n\tbool unk1 = {}\n)", client, unk1));

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
