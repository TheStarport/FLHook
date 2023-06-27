#pragma once

struct ServerStats
{
        struct Player
        {
                std::wstring playerName;
                std::wstring systemName;
                std::wstring systemNick;
                std::wstring ipAddress;
                uint clientId;
        };

        std::vector<Player> players;
        uint serverLoad;
        uint memoryUsage;
        bool npcsEnabled;

        Serialize(Player, systemName, systemNick, ipAddress, clientId);
        Serialize(ServerStats, players, serverLoad, memoryUsage, npcsEnabled);
};
