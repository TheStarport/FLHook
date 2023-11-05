#include "PCH.hpp"

#include "API/API.hpp"
#include "Global.hpp"

#include <API/Utils/IniUtils.hpp>

namespace IServerImplHook
{
    std::wstring g_CharBefore;
    bool CharacterSelect__Inner(const CHARACTER_ID& cid, ClientId client)
    {
        try
        {
            const auto info = &ClientInfo[client];
            auto charName = Hk::Client::GetCharacterNameByID(client).Unwrap();
            g_CharBefore = !charName.empty() ? charName : L"";
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

    void CharacterSelect__InnerAfter([[maybe_unused]] const CHARACTER_ID& charId, unsigned int client)
    {
        TRY_HOOK
        {
            auto& info = ClientInfo[client];
            info.characterFile = StringUtils::stows(charId.charFilename);
            const std::wstring charName = Hk::Client::GetCharacterNameByID(client).Handle();

            if (g_CharBefore != charName)
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

                auto& ci = ClientInfo[client];

                ci.formationNumber1 = distNum(rng);
                ci.formationNumber2 = distNum(rng);
                ci.formationTag = conf->callsign.allowedFormations[distForm(rng)];
            }
        }
        CATCH_HOOK({})
    }

    void __stdcall CharacterSelect(const CHARACTER_ID& cid, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"CharacterSelect(\n\tClientId client = {}\n)", client));

        const std::wstring charName = StringUtils::stows(cid.charFilename);
        const auto skip = CallPlugins(&Plugin::OnCharacterSelect, client, std::wstring_view(charName));

        CHECK_FOR_DISCONNECT;

        if (const bool innerCheck = CharacterSelect__Inner(cid, client); !innerCheck)
        {
            return;
        }
        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.CharacterSelect(cid, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        CharacterSelect__InnerAfter(cid, client);

        CallPlugins(&Plugin::OnCharacterSelectAfter, client, std::wstring_view(charName));
    }

    void __stdcall CreateNewCharacter(const SCreateCharacterInfo& _genArg1, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"CreateNewCharacter(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnCharacterCreation, client, _genArg1); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.CreateNewCharacter(_genArg1, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnCharacterCreationAfter, client, _genArg1);
    }

    void __stdcall DestroyCharacter(const CHARACTER_ID& cid, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"DestroyCharacter(\n\tClientId client = {}\n)", client));

        const std::wstring charName = StringUtils::stows(cid.charFilename);

        if (const auto skip = CallPlugins(&Plugin::OnCharacterDelete, client, std::wstring_view(charName)); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.DestroyCharacter(cid, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnCharacterDeleteAfter, client, std::wstring_view(charName));
    }

    void __stdcall RequestRankLevel(ClientId client, uint _genArg1, int _genArg2)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"RequestRankLevel(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));

        if (const auto skip = CallPlugins(&Plugin::OnRequestRankLevel, client, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestRankLevel(client, (uchar*)_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestRankLevelAfter, client, _genArg1, _genArg2);
    }

    void __stdcall RequestPlayerStats(ClientId client, uint _genArg1, int _genArg2)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"RequestPlayerStats(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));

        if (const auto skip = CallPlugins(&Plugin::OnRequestPlayerStats, client, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestPlayerStats(client, (uchar*)_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestPlayerStats, client, _genArg1, _genArg2);
    }

    bool CharacterInfoReq__Inner(ClientId client, bool)
    {
        TRY_HOOK
        {
            if (!ClientInfo[client].charMenuEnterTime)
            {
                ClientInfo[client].characterName = Hk::Client::GetCharacterNameByID(client).Unwrap();
            }
            else
            {
                // pushed f1
                uint shipId = 0;
                pub::Player::GetShip(client, shipId);
                if (shipId)
                {
                    // in space
                    ClientInfo[client].tmF1Time = TimeUtils::UnixMilliseconds() + FLHookConfig::i()->general.antiF1;
                    return false;
                }
            }
        }
        CATCH_HOOK({})

        return true;
    }

    bool CharacterInfoReq__Catch(ClientId client, bool)
    {
        // AddKickLog(client, "Corrupt charfile?");
        Hk::Player::Kick(client);
        return false;
    }

    void __stdcall CharacterInfoReq(ClientId client, bool _genArg1)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"CharacterInfoReq(\n\tClientId client = {}\n\tbool _genArg1 = {}\n)", client, _genArg1));

        const auto skip = CallPlugins(&Plugin::OnCharacterInfoRequest, client, _genArg1);

        CHECK_FOR_DISCONNECT;

        if (const bool innerCheck = CharacterInfoReq__Inner(client, _genArg1); !innerCheck)
        {
            return;
        }
        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.CharacterInfoReq(client, _genArg1); }
            CALL_SERVER_POSTAMBLE(CharacterInfoReq__Catch(client, _genArg1), );
        }

        CallPlugins(&Plugin::OnCharacterInfoRequestAfter, client, _genArg1);
    }

} // namespace IServerImplHook
