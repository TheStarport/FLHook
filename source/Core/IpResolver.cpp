#include "PCH.hpp"

#include <ws2tcpip.h>

#include "Core/IpResolver.hpp"
#include "Defs/FLHookConfig.hpp"
#include "API/FLHook/ClientList.hpp"

#include "Core/ExceptionHandler.hpp"
#include "Exceptions/StopProcessingException.hpp"

void IpResolver::ThreadResolver()
{
    TryHook
    {
        while (true)
        {
            mutex.lock();
            std::vector<ResolvedIp> ips = resolveIPs;
            resolveIPs.clear();
            mutex.unlock();

            for (auto& [client, connects, IP, hostname] : ips)
            {
                SOCKADDR_IN addr{ AF_INET, 2302, {}, { 0 } };
                InetPtonW(AF_INET, IP.c_str(), &addr.sin_addr);

                static std::wstring hostBuffer;
                hostBuffer.resize(255);

                GetNameInfoW(reinterpret_cast<const SOCKADDR*>(&addr), sizeof addr, hostBuffer.data(), hostBuffer.size(), nullptr, 0, 0);

                hostname = hostBuffer;
            }

            mutex.lock();
            for (auto& ip : ips)
            {
                if (ip.hostname.length())
                {
                    resolveIPsResult.push_back(ip);
                }
            }

            mutex.unlock();

            Sleep(50);
        }
    }
    CatchHook({});
}

void IpResolver::TimerCheckResolveResults()
{
    TryHook
    {
        std::scoped_lock lock(mutex);
        for (const auto& [client, connects, IP, hostname] : resolveIPsResult)
        {
            auto cl = ClientId(client);
            auto& data = cl.GetData();
            if (connects != data.connects)
            {
                continue; // outdated
            }

            // check if banned
            for (const auto config = FLHook::GetConfig(); const auto& ban : config->bans.banWildcardsAndIPs)
            {
                if (wildcards::match(hostname, ban))
                {
                    // AddKickLog(ip.client, StringUtils::wstos(std::format(L"IP/hostname ban({} matches {})", ip.hostname.c_str(), ban.c_str())));
                    if (config->bans.banAccountOnMatch)
                    {
                        cl.GetAccount().Unwrap().Ban(true);
                    }

                    cl.Kick();
                }
            }
            data.hostname = hostname;
        }

        resolveIPsResult.clear();
    }
    CatchHook({})
}
