#pragma once

struct ServerStats
{
        struct Player
        {
                std::string playerName;
                std::string systemName;
                std::string systemNick;
                std::string ipAddress;
                uint clientId;
        };

        std::vector<Player> players;
        uint serverLoad;
        uint memoryUsage;
        bool npcsEnabled;
};
