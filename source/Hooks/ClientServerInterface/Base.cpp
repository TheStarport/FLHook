#include "PCH.hpp"

#include "API/API.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

void BaseEnterInner([[maybe_unused]] uint baseId, [[maybe_unused]] ClientId client)
{
    // TODO: implement base enter event
}

void BaseEnterInnerAfter([[maybe_unused]] uint baseId, ClientId client)
{
    TryHook
    {
        // adjust cash, this is necessary when cash was added while use was in
        // charmenu/had other char selected
        std::wstring charName = StringUtils::ToLower(client.GetCharacterName().Unwrap());
        for (const auto& i : ClientInfo::At(client).moneyFix)
        {
            if (i.character == charName)
            {
                Hk::Player::AddCash(charName, i.amount);
                ClientInfo::At(client).moneyFix.remove(i);
                break;
            }
        }

        // anti base-idle
        ClientInfo::At(client).baseEnterTime = static_cast<uint>(time(nullptr));

        // print to log if the char has too much money
        if (const auto value = Hk::Player::GetShipValue((const wchar_t*)Players.GetActiveCharacterName(client)).Raw();
            value.has_value() && value.value() > 2000000000)
        {
            const std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
            Logger::i()->Log(LogLevel::Trace, std::format(L"Possible corrupt ship charname={} asset_value={}", charname, value.value()));
        }
    }
    CatchHook({})
}
void __stdcall IServerImplHook::BaseEnter(uint baseId, ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"BaseEnter(\n\tuint baseId = {}\n\tClientId client = {}\n)", baseId, client));

    const auto skip = CallPlugins(&Plugin::OnBaseEnter, baseId, client);

    CheckForDisconnect;

    BaseEnterInner(baseId, client);

    if (!skip)
    {
        CallServerPreamble { Server.BaseEnter(baseId, client); }
        CallServerPostamble(true, );
    }
    BaseEnterInnerAfter(baseId, client);

    CallPlugins(&Plugin::OnBaseEnterAfter, baseId, client);
}
void BaseExitInner(uint baseId, ClientId client)
{
    TryHook
    {
        ClientInfo::At(client).baseEnterTime = 0;
        ClientInfo::At(client).lastExitedBaseId = baseId;
    }
    CatchHook({})
}

void BaseExitInnerAfter([[maybe_unused]] uint baseId, [[maybe_unused]] ClientId client)
{
    // TODO: implement base exit event
}
void __stdcall IServerImplHook::BaseExit(uint baseId, ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"BaseExit(\n\tuint baseId = {}\n\tClientId client = {}\n)", baseId, client));

    const auto skip = CallPlugins(&Plugin::OnBaseExit, baseId, client);

    CheckForDisconnect;

    BaseExitInner(baseId, client);

    if (!skip)
    {
        CallServerPreamble { Server.BaseExit(baseId, client); }
        CallServerPostamble(true, );
    }
    BaseExitInnerAfter(baseId, client);

    CallPlugins(&Plugin::OnBaseExitAfter, baseId, client);
}

void __stdcall IServerImplHook::BaseInfoRequest(unsigned int unk1, unsigned int unk2, bool unk3)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"BaseInfoRequest(\n\tunsigned int unk1 = {}\n\tunsigned int unk2 = {}\n\tbool unk3 = {}\n)", unk1, unk2, unk3));

    if (const auto skip = CallPlugins(&Plugin::OnRequestBaseInfo, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.BaseInfoRequest(unk1, unk2, unk3); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestBaseInfoAfter, unk1, unk2, unk3);
}

void __stdcall IServerImplHook::Dock([[maybe_unused]] const uint& genArg1, [[maybe_unused]] const uint& genArg2) {}
