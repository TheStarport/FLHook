#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void __stdcall SPScanCargo(const uint& _genArg1, const uint& _genArg2, uint _genArg3)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"SPScanCargo(\n\tuint const& _genArg1 = {}\n\tuint const& _genArg2 = {}\n\tuint _genArg3 = {}\n)", _genArg1, _genArg2, _genArg3));

        if (const auto skip = CallPlugins(&Plugin::OnSpScanCargo, _genArg1, _genArg2, _genArg3); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SPScanCargo(_genArg1, _genArg2, _genArg3); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnSpScanCargoAfter, _genArg1, _genArg2, _genArg3);
    }

    void __stdcall ReqAddItem(uint goodId, const char* hardpoint, int count, float status, bool mounted, ClientId client)
    {
        const std::wstring hp = StringUtils::stows(hardpoint);
        Logger::i()->Log(LogLevel::Trace,
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
            CALL_SERVER_PREAMBLE { Server.ReqAddItem(goodId, hardpoint, count, status, mounted, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestAddItemAfter, client, goodId, std::wstring_view(hp), count, status, mounted);
    }

    void __stdcall ReqRemoveItem(ushort slotId, int count, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace,
                         std::format(L"ReqRemoveItem(\n\tushort slotId = {}\n\tint count = {}\n\tClientId client = {}\n)", slotId, count, client));

        if (const auto skip = CallPlugins(&Plugin::OnRequestRemoveItem, client, slotId, count); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqRemoveItem(slotId, count, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestRemoveItemAfter, client, slotId, count);
    }

    void __stdcall ReqModifyItem(ushort slotId, const char* hardpoint, int count, float status, bool mounted, ClientId client)
    {
        std::wstring hp = StringUtils::stows(hardpoint);
        Logger::i()->Log(LogLevel::Trace,
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
            CALL_SERVER_PREAMBLE { Server.ReqModifyItem(slotId, hardpoint, count, status, mounted, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestModifyItemAfter, client, slotId, std::wstring_view(hp), count, status, mounted);
    }

    void __stdcall JettisonCargo(ClientId client, const XJettisonCargo& jc)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"JettisonCargo(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnCargoJettison, client, jc); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.JettisonCargo(client, jc); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnCargoJettisonAfter, client, jc);
    }

    void __stdcall TractorObjects(ClientId client, const XTractorObjects& to)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"TractorObjects(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnTractorObjects, client, to); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.TractorObjects(client, to); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnTractorObjectsAfter, client, to);
    }

} // namespace IServerImplHook
