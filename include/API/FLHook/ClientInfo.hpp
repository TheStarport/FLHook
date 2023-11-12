#pragma once

class ClientInfo
{
    public:
        void SaveAccountData() const;

        // JSON object of various keys and custom data. Some of these will map directly on to members of this class, others are by accessor only.
        nlohmann::json accountData;

        uint client;
        std::wstring characterName;
        std::wstring characterFile;

        uint ship;
        uint shipOld;
        mstime spawnTime;

        DamageList dmgLast;

        std::list<MoneyFix> moneyFix;

        // anticheat
        uint tradePartner;

        // change cruise disruptor behaviour
        bool cruiseActivated;
        bool thrusterActivated;
        bool engineKilled;
        bool tradelane;

        // idle kicks
        uint baseEnterTime;
        uint charMenuEnterTime;

        // msg, wait and kick
        mstime kickTime;

        // eventmode
        uint lastExitedBaseId;
        bool disconnected;

        // f1 laming
        mstime f1Time;
        mstime timeDisconnect;

        // ignore usercommand
        std::list<IgnoreInfo> ignoreInfoList;

        // user settings
        DIEMSGTYPE dieMsg;
        CHATSIZE dieMsgSize;
        CHATSTYLE dieMsgStyle;
        CHATSIZE chatSize;
        CHATSTYLE chatStyle;

        // MultiKillMessages
        uint killsInARow;

        // bans
        uint connects; // incremented when player connects

        // Group
        uint groupId;

        // other
        std::wstring hostname;

        bool spawnProtected;

        // Your randomly assigned formation tag, e.g. Navy Lambda 1-6
        uint formationNumber1;
        uint formationNumber2;
        uint formationTag;

        static ClientInfo& At(ClientId client)
        {
            if (client.GetValue() > MaxClientId)
            {
                static std::string err;
                err = std::format("Attempted to access client info with an invalid id. ID: {}", client);
                throw std::out_of_range(err.c_str());
            }

            return clients[client.GetValue()];
        }
};
