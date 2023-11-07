#pragma once

class FLHook;
class IpResolver
{
        friend FLHook;

        struct ResolvedIp
        {
                uint client;
                uint connects;
                std::wstring ip;
                std::wstring hostname;
        };

        inline static std::mutex mutex;
        inline static std::vector<ResolvedIp> resolveIPs;
        inline static std::vector<ResolvedIp> resolveIPsResult;
        inline static std::thread resolveThread;

        static void ThreadResolver();
        static void TimerCheckResolveResults();
};
