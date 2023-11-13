#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SpScanCargo(const uint& unk1, const uint& unk2, uint unk3)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"SPScanCargo(\n\tuint const& unk1 = {}\n\tuint const& unk2 = {}\n\tuint unk3 = {}\n)", unk1, unk2, unk3));

    if (const auto skip = CallPlugins(&Plugin::OnSpScanCargo, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.SPScanCargo(unk1, unk2, unk3); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpScanCargoAfter, unk1, unk2, unk3);
}

void __stdcall IServerImplHook::ReqAddItem(uint goodId, const char* hardpoint, int count, float status, bool mounted, ClientId client)
{
    const std::wstring hp = StringUtils::stows(hardpoint);
    FLHook::GetLogger().Log(LogLevel::Trace,
                     std::format(L"ReqAddItem(\n\tuint goodId = {}\n\tchar const* hardpoint = {}\n\tint count = {}\n\tfloat status = "
                                 L"{}\n\tbool mounted = {}\n\tClientId client = {}\n)",
                                 goodId,
                                 StringUtils::stows(std::string(hardpoint)),
                                 count,
                                 status,
                                 mounted,
                                 client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestAddItem, client, goodId, std::wstring_view(hp), count, status, mounted); !skip)
    {
        CallServerPreamble { Server.ReqAddItem(goodId, hardpoint, count, status, mounted, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestAddItemAfter, client, goodId, std::wstring_view(hp), count, status, mounted);
}

void __stdcall IServerImplHook::ReqRemoveItem(ushort slotId, int count, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"ReqRemoveItem(\n\tushort slotId = {}\n\tint count = {}\n\tClientId client = {}\n)", slotId, count, client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestRemoveItem, client, slotId, count); !skip)
    {
        CallServerPreamble { Server.ReqRemoveItem(slotId, count, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestRemoveItemAfter, client, slotId, count);
}

void __stdcall IServerImplHook::ReqModifyItem(ushort slotId, const char* hardpoint, int count, float status, bool mounted, ClientId client)
{
    std::wstring hp = StringUtils::stows(hardpoint);
    FLHook::GetLogger().Log(LogLevel::Trace,
                     std::format(L"ReqModifyItem(\n\tushort slotId = {}\n\tchar const* hardpoint = {}\n\tint count = {}\n\tfloat status = "
                                 "{}\n\tbool mounted = {}\n\tClientId client = {}\n)",
                                 slotId,
                                 hp,
                                 count,
                                 status,
                                 mounted,
                                 client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestModifyItem, client, slotId, std::wstring_view(hp), count, status, mounted); !skip)
    {
        CallServerPreamble { Server.ReqModifyItem(slotId, hardpoint, count, status, mounted, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestModifyItemAfter, client, slotId, std::wstring_view(hp), count, status, mounted);
}

void __stdcall IServerImplHook::JettisonCargo(ClientId client, const XJettisonCargo& jc)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"JettisonCargo(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnCargoJettison, client, jc); !skip)
    {
        CallServerPreamble { Server.JettisonCargo(client, jc); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnCargoJettisonAfter, client, jc);
}

void __stdcall IServerImplHook::TractorObjects(ClientId client, const XTractorObjects& to)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"TractorObjects(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnTractorObjects, client, to); !skip)
    {
        CallServerPreamble { Server.TractorObjects(client, to); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnTractorObjectsAfter, client, to);
}
