#pragma once

struct CargoInfo
{
        uint id;
        int count;
        uint archId;
        float status;
        bool mission;
        bool mounted;
        CacheString hardpoint;
};

struct LaunchData
{
        CEqObj* obj;
        Vector pos;
        Matrix orientation;
        int dock;
};

// money stuff
struct MoneyFix
{
        std::wstring character;
        uint amount;

        bool operator==(const MoneyFix mf1) const
        {
            if (character != mf1.character)
            {
                return true;
            }

            return false;
        };
};

// ignore
struct IgnoreInfo
{
        std::wstring character;
        std::wstring flags;
};

// taken from directplay
typedef struct _DPN_CONNECTION_INFO
{
        DWORD size;
        DWORD roundTripLatencyMS;
        DWORD throughputBPS;
        DWORD peakThroughputBPS;
        DWORD bytesSentGuaranteed;
        DWORD packetsSentGuaranteed;
        DWORD bytesSentNonGuaranteed;
        DWORD packetsSentNonGuaranteed;
        DWORD bytesRetried;
        DWORD packetsRetried;
        DWORD bytesDropped;
        DWORD packetsDropped;
        DWORD messagesTransmittedHighPriority;
        DWORD messagesTimedOutHighPriority;
        DWORD messagesTransmittedNormalPriority;
        DWORD messagesTimedOutNormalPriority;
        DWORD messagesTransmittedLowPriority;
        DWORD messagesTimedOutLowPriority;
        DWORD bytesReceivedGuaranteed;
        DWORD packetsReceivedGuaranteed;
        DWORD bytesReceivedNonGuaranteed;
        DWORD packetsReceivedNonGuaranteed;
        DWORD messagesReceived;
} DPN_CONNECTION_INFO, *PDPN_CONNECTION_INFO;

struct PlayerInfo
{
        uint client;
        std::wstring character;
        std::wstring baseName;
        std::wstring systemName;
        uint system;
        uint ship;
        DPN_CONNECTION_INFO connectionInfo;
        std::wstring IP;
        std::wstring hostname;
};

struct DataMarketItem
{
        uint archId;
        float rep;
};

struct Light
{
        std::wstring nickname;
        uint archId;
        float bulbSize;
        float glowSize;
        float intensity;
        Vector glowColor;
        Vector color;
        Vector minColor;
        bool dockingLight;
        bool alwaysOn;
        float flareConeMin;
        float flareConeMax;
        int lightSourceCone;

        bool blinks;
        float delay;
        float blinkDuration;
};

template <class Processor>
struct CommandInfo
{
        std::vector<std::wstring_view> cmd;
        void (*func)(Processor* cl, std::vector<std::wstring>& params);
        std::wstring_view usage;
        std::wstring_view description;
};
