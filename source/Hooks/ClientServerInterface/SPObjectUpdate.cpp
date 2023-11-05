#include "PCH.hpp"

#include "API/FLServer/Player.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    bool SPObjUpdate__Inner(const SSPObjUpdateInfo& ui, ClientId client)
    {
        // NAN check
        if (isnan(ui.pos.x) || isnan(ui.pos.y) || isnan(ui.pos.z) || isnan(ui.dir.w) || isnan(ui.dir.x) || isnan(ui.dir.y) || isnan(ui.dir.z) ||
            isnan(ui.throttle))
        {
            Logger::i()->Log(LogLevel::Trace, std::format(L"NAN found in SPObjUpdate for id={}", client));
            Hk::Player::Kick(client);
            return false;
        }

        // Denormalized check
        if (const float n = ui.dir.w * ui.dir.w + ui.dir.x * ui.dir.x + ui.dir.y * ui.dir.y + ui.dir.z * ui.dir.z; n > 1.21f || n < 0.81f)
        {
            Logger::i()->Log(LogLevel::Trace, std::format(L"Non-normalized quaternion found in SPObjUpdate for id={}", client));
            Hk::Player::Kick(client);
            return false;
        }

        // Far check
        if (abs(ui.pos.x) > 1e7f || abs(ui.pos.y) > 1e7f || abs(ui.pos.z) > 1e7f)
        {
            Logger::i()->Log(LogLevel::Trace, std::format(L"Ship position out of bounds in SPObjUpdate for id={}", client));
            Hk::Player::Kick(client);
            return false;
        }

        return true;
    }

    void __stdcall SPObjUpdate(const SSPObjUpdateInfo& ui, ClientId client)
    {
        const auto skip = CallPlugins(&Plugin::OnSpObjectUpdate, client, ui);

        CHECK_FOR_DISCONNECT;

        if (const bool innerCheck = SPObjUpdate__Inner(ui, client); !innerCheck)
        {
            return;
        }
        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.SPObjUpdate(ui, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnSpObjectUpdateAfter, client, ui);
    }
} // namespace IServerImplHook
