#include "PCH.hpp"

#include "API/API.hpp"

#include "Core/ClientServerInterface.hpp"


#include <API/Utils/IniUtils.hpp>

bool IServerImplHook::CharacterSelectInner(const CHARACTER_ID& cid, ClientId client)
{
    try
    {
        const auto info = &ClientInfo::At(client);
        auto charName = client.GetCharacterName().Unwrap();
        charBefore = !charName.empty() ? charName : L"";
        info->lastExitedBaseId = 0;
        info->tradePartner = 0;
        info->characterName = charName;
    }
    catch (...)
    {
        // AddKickLog(client, "Corrupt character file?");
        Hk::Player::Kick(client);
        return false;
    }

    Hk::IniUtils::i()->CharacterSelect(cid, client);
    return true;
}

void IServerImplHook::CharacterSelectInnerAfter([[maybe_unused]] const CHARACTER_ID& charId, unsigned int client)
{
    TryHook
    {
        auto& info = ClientInfo::At(client);
        info.characterFile = StringUtils::stows(charId.charFilename);
        const std::wstring charName = client.GetCharacterName().Handle();

        if (charBefore != charName)
        {
            CallPlugins(&Plugin::OnLoadCharacterSettings, client, std::wstring_view(charName));

            if (FLHookConfig::i()->userCommands.userCmdHelp)
            {
                PrintUserCmdText(client,
                                 L"To get a list of available commands, type "
                                 L"\"/help\" in chat.");
            }

            int hold;
            auto cargoList = Hk::Player::EnumCargo(client, hold).Raw();
            if (cargoList.has_error())
            {
                Hk::Player::Kick(client);
                return;
            }

            for (const auto& cargo : cargoList.value())
            {
                if (cargo.count < 0)
                {
                    // AddCheaterLog(charName, "Negative good-count, likely to have cheated in the past");

                    Hk::Chat::MsgU(std::format(L"Possible cheating detected: {}", charName));
                    Hk::Player::Ban(client, true);
                    Hk::Player::Kick(client);
                    return;
                }
            }

            // event
            const CAccount* acc = Players.FindAccountFromClientID(client);
            std::wstring dir = Hk::Client::GetAccountDirName(acc);
            auto pi = Hk::Admin::GetPlayerInfo(client, false);

            MailManager::i()->SendMailNotification(client);

            // Assign their random formation id.
            // Numbers are between 0-20 (inclusive)
            // Formations are between 1-29 (inclusive)
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> distNum(1, 20);
            const auto* conf = FLHookConfig::c();
            std::uniform_int_distribution<std::mt19937::result_type> distForm(0, conf->callsign.allowedFormations.size() - 1);

            auto& ci = ClientInfo::At(client);

            ci.formationNumber1 = distNum(rng);
            ci.formationNumber2 = distNum(rng);
            ci.formationTag = conf->callsign.allowedFormations[distForm(rng)];
        }
    }
    CatchHook({})
}

void __stdcall IServerImplHook::CharacterSelect(const CHARACTER_ID& cid, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"CharacterSelect(\n\tClientId client = {}\n)", client));

    const std::wstring charName = StringUtils::stows(cid.charFilename);
    const auto skip = CallPlugins(&Plugin::OnCharacterSelect, client, std::wstring_view(charName));

    CheckForDisconnect;

    if (const bool innerCheck = CharacterSelectInner(cid, client); !innerCheck)
    {
        return;
    }
    if (!skip)
    {
        CallServerPreamble { Server.CharacterSelect(cid, client); }
        CallServerPostamble(true, );
    }
    CharacterSelectInnerAfter(cid, client);

    CallPlugins(&Plugin::OnCharacterSelectAfter, client, std::wstring_view(charName));
}

void __stdcall IServerImplHook::CreateNewCharacter(const SCreateCharacterInfo& unk1, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"CreateNewCharacter(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnCharacterCreation, client, unk1); !skip)
    {
        CallServerPreamble { Server.CreateNewCharacter(unk1, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnCharacterCreationAfter, client, unk1);
}

void __stdcall IServerImplHook::DestroyCharacter(const CHARACTER_ID& cid, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"DestroyCharacter(\n\tClientId client = {}\n)", client));

    const std::wstring charName = StringUtils::stows(cid.charFilename);

    if (const auto skip = CallPlugins(&Plugin::OnCharacterDelete, client, std::wstring_view(charName)); !skip)
    {
        CallServerPreamble { Server.DestroyCharacter(cid, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnCharacterDeleteAfter, client, std::wstring_view(charName));
}

void __stdcall IServerImplHook::RequestRankLevel(ClientId client, uint unk1, int unk2)
{
    FLHook::GetLogger().Log(LogLevel::Trace,
                     std::format(L"RequestRankLevel(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnRequestRankLevel, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestRankLevel(client, (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestRankLevelAfter, client, unk1, unk2);
}

void __stdcall IServerImplHook::RequestPlayerStats(ClientId client, uint unk1, int unk2)
{
    FLHook::GetLogger().Log(LogLevel::Trace,
                     std::format(L"RequestPlayerStats(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnRequestPlayerStats, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestPlayerStats(client, (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestPlayerStats, client, unk1, unk2);
}

bool CharacterInfoReqInner(ClientId client, bool)
{
    TryHook
    {
        if (!ClientInfo::At(client).charMenuEnterTime)
        {
            ClientInfo::At(client).characterName = client.GetCharacterName().Unwrap();
        }
        else
        {
            // pushed f1
            uint shipId = 0;
            pub::Player::GetShip(client, shipId);
            if (shipId)
            {
                // in space
                ClientInfo::At(client).tmF1Time = TimeUtils::UnixMilliseconds() + FLHookConfig::i()->general.antiF1;
                return false;
            }
        }
    }
    CatchHook({})

    return true;
}

bool CharacterInfoReqCatch(ClientId client, bool)
{
    // AddKickLog(client, "Corrupt charfile?");
    Hk::Player::Kick(client);
    return false;
}

void __stdcall IServerImplHook::CharacterInfoReq(ClientId client, bool unk1)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"CharacterInfoReq(\n\tClientId client = {}\n\tbool unk1 = {}\n)", client, unk1));

    const auto skip = CallPlugins(&Plugin::OnCharacterInfoRequest, client, unk1);

    CheckForDisconnect;

    if (const bool innerCheck = CharacterInfoReqInner(client, unk1); !innerCheck)
    {
        return;
    }

    if (!skip)
    {
        CallServerPreamble { Server.CharacterInfoReq(client, unk1); }
        CallServerPostamble(CharacterInfoReqCatch(client, unk1), );
    }

    CallPlugins(&Plugin::OnCharacterInfoRequestAfter, client, unk1);
}
