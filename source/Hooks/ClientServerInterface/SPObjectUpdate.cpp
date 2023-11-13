#include "PCH.hpp"

#include "API/FLServer/Player.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

bool SPObjUpdateInner(const SSPObjUpdateInfo& ui, ClientId client)
{
    // NAN check
    if (isnan(ui.pos.x) || isnan(ui.pos.y) || isnan(ui.pos.z) || isnan(ui.dir.w) || isnan(ui.dir.x) || isnan(ui.dir.y) || isnan(ui.dir.z) || isnan(ui.throttle))
    {
        FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"NAN found in SPObjUpdate for id={}", client));
        client.Kick(L"Possible cheating detected");
        return false;
    }

    // Denormalized check
    if (const float n = ui.dir.w * ui.dir.w + ui.dir.x * ui.dir.x + ui.dir.y * ui.dir.y + ui.dir.z * ui.dir.z; n > 1.21f || n < 0.81f)
    {
        FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"Non-normalized quaternion found in SPObjUpdate for id={}", client));
        client.Kick(L"Possible cheating detected");
        return false;
    }

    // Far check
    if (abs(ui.pos.x) > 1e7f || abs(ui.pos.y) > 1e7f || abs(ui.pos.z) > 1e7f)
    {
        FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"Ship position out of bounds in SPObjUpdate for id={}", client));
        client.Kick(L"Possible cheating detected");
        return false;
    }

    return true;
}

void __stdcall IServerImplHook::SpObjUpdate(const SSPObjUpdateInfo& ui, ClientId client)
{
    const auto skip = CallPlugins(&Plugin::OnSpObjectUpdate, client, ui);

    CheckForDisconnect;

    if (const bool innerCheck = SPObjUpdateInner(ui, client); !innerCheck)
    {
        return;
    }
    if (!skip)
    {
        CallServerPreamble { Server.SPObjUpdate(ui, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpObjectUpdateAfter, client, ui);
}
