#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Types/ClientId.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Logger.hpp"

void DisConnectInner(ClientId client, EFLConnection)
{
    auto& data = client.GetData();
    if (!data.disconnected)
    {
        data.disconnected = true;
        data.moneyFix.clear();
        data.tradePartner = ClientId();

        // TODO: implement event for disconnect
    }
}

void __stdcall IServerImplHook::DisConnect(ClientId client, EFLConnection conn)
{
    const auto msg = std::format(L"DisConnect(\n\tClientId client = {}\n)", client);

    FLHook::GetLogger().Log(LogLevel::Trace, msg);

    const auto skip = CallPlugins(&Plugin::OnDisconnect, client, conn);

    DisConnectInner(client, conn);

    if (!skip)
    {
        {
            static PerfTimer timer(StringUtils::stows(__FUNCTION__), 100);
            timer.Start();
            TryHook
            {
                {
                    Server.DisConnect(client.GetValue(), conn);
                }
            }
        }
        catch ([[maybe_unused]] SehException& exc)
        {
            {
                FLHook::GetLogger().Log(LogLevel::Err, std::format(L"Exception in {} on server call", StringUtils::stows(__FUNCTION__)));
                bool ret = true;
                if (!ret)
                {
                    timer.Stop();
                    return;
                }
            };
        }
        catch ([[maybe_unused]] const StopProcessingException&) {}
        catch (const GameException& ex)
        {
            FLHook::GetLogger().Log(LogLevel::Info, ex.Msg());
            {
                FLHook::GetLogger().Log(LogLevel::Err, std::format(L"Exception in {} on server call", StringUtils::stows(__FUNCTION__)));
                bool ret = true;
                if (!ret)
                {
                    timer.Stop();
                    return;
                }
            };
        }
        catch ([[maybe_unused]] std::exception& exc)
        {
            {
                FLHook::GetLogger().Log(LogLevel::Err, std::format(L"Exception in {} on server call", StringUtils::stows(__FUNCTION__)));
                bool ret = true;
                if (!ret)
                {
                    timer.Stop();
                    return;
                }
            };
        }
        catch (...)
        {
            {
                FLHook::GetLogger().Log(LogLevel::Err, std::format(L"Exception in {} on server call", StringUtils::stows(__FUNCTION__)));
                bool ret = true;
                if (!ret)
                {
                    timer.Stop();
                    return;
                }
            };
        }
        timer.Stop();
    };
}

CallPlugins(&Plugin::OnDisconnectAfter, client, conn);
}
