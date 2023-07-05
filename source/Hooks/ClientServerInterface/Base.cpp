#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void BaseEnter__Inner([[maybe_unused]] uint baseId, [[maybe_unused]] ClientId client)
    {
        // TODO: implement base enter event
    }

    void BaseEnter__InnerAfter([[maybe_unused]] uint baseId, ClientId client)
    {
        TRY_HOOK
        {
            // adjust cash, this is necessary when cash was added while use was in
            // charmenu/had other char selected
            std::wstring charName = StringUtils::ToLower(Hk::Client::GetCharacterNameByID(client).Unwrap());
            for (const auto& i : ClientInfo[client].moneyFix)
            {
                if (i.character == charName)
                {
                    Hk::Player::AddCash(charName, i.amount);
                    ClientInfo[client].moneyFix.remove(i);
                    break;
                }
            }

            // anti base-idle
            ClientInfo[client].baseEnterTime = static_cast<uint>(time(nullptr));

            // print to log if the char has too much money
            if (const auto value = Hk::Player::GetShipValue((const wchar_t*)Players.GetActiveCharacterName(client)).Raw();
                value.has_value() && value.value() > 2000000000)
            {
                const std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
                Logger::i()->Log(LogLevel::Trace, std::format(L"Possible corrupt ship charname={} asset_value={}", charname, value.value()));
            }
        }
        CATCH_HOOK({})
    }
    void __stdcall BaseEnter(uint baseId, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"BaseEnter(\n\tuint baseId = {}\n\tClientId client = {}\n)", baseId, client));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseEnter, baseId, client);

        CHECK_FOR_DISCONNECT;

        BaseEnter__Inner(baseId, client);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.BaseEnter(baseId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        BaseEnter__InnerAfter(baseId, client);

        CallPluginsAfter(HookedCall::IServerImpl__BaseEnter, baseId, client);
    }
    void BaseExit__Inner(uint baseId, ClientId client)
    {
        TRY_HOOK
        {
            ClientInfo[client].baseEnterTime = 0;
            ClientInfo[client].lastExitedBaseId = baseId;
        }
        CATCH_HOOK({})
    }

    void BaseExit__InnerAfter([[maybe_unused]] uint baseId, [[maybe_unused]] ClientId client)
    {
        // TODO: implement base exit event
    }
    void __stdcall BaseExit(uint baseId, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"BaseExit(\n\tuint baseId = {}\n\tClientId client = {}\n)", baseId, client));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseExit, baseId, client);

        CHECK_FOR_DISCONNECT;

        BaseExit__Inner(baseId, client);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.BaseExit(baseId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        BaseExit__InnerAfter(baseId, client);

        CallPluginsAfter(HookedCall::IServerImpl__BaseExit, baseId, client);
    }

    void __stdcall BaseInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(
                L"BaseInfoRequest(\n\tunsigned int _genArg1 = {}\n\tunsigned int _genArg2 = {}\n\tbool _genArg3 = {}\n)", _genArg1, _genArg2, _genArg3));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseInfoRequest, _genArg1, _genArg2, _genArg3); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.BaseInfoRequest(_genArg1, _genArg2, _genArg3); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__BaseInfoRequest, _genArg1, _genArg2, _genArg3);
    }

    void __stdcall Dock([[maybe_unused]] const uint& genArg1, [[maybe_unused]] const uint& genArg2) {}

} // namespace IServerImplHook
