#pragma once

#include "Defs/Database/Account.hpp"

struct ClientData
{
        friend AccountManager;

        bsoncxx::document::view characterData;
        ClientId id;
        Account* account = nullptr;
        std::wstring_view characterName{};
        PlayerData* playerData = nullptr;
        bool isValid = false;

        ShipId ship{};
        ShipId shipOld{};
        mstime spawnTime = 0;

        DamageList dmgLast{};

        std::list<MoneyFix> moneyFix{};

        // anticheat
        ClientId tradePartner{};

        // change cruise disruptor behaviour
        bool cruiseActivated = false;
        bool thrusterActivated = false;
        bool engineKilled = false;
        bool inTradelane = false;

        // idle kicks
        BaseId baseId{};
        uint baseEnterTime = 0;
        uint charMenuEnterTime = 0;

        // msg, wait and kick
        mstime kickTime = 0;

        // eventmode
        uint lastExitedBaseId = 0;
        bool disconnected = false;

        // f1 laming
        mstime f1Time = 0;
        mstime timeDisconnect = 0;

        // ignore usercommand
        std::list<IgnoreInfo> ignoreInfoList{};

        // Chat Styles

        DieMsgType dieMsg = DieMsgType::All;
        ChatSize dieMsgSize = ChatSize::Default;
        ChatStyle dieMsgStyle = ChatStyle::Default;
        ChatSize chatSize = ChatSize::Default;
        ChatStyle chatStyle = ChatStyle::Default;

        // bans
        uint connects = 0; // incremented when player connects

        // Group
        uint groupId = 0;

        // other
        std::wstring hostname{};

        bool spawnProtected = false;

        // Your randomly assigned formation tag, e.g. Navy Lambda 1-6
        uint formationNumber1 = 0;
        uint formationNumber2 = 0;
        AllowedFormation formationTag = AllowedFormation::Alpha;

        ClientData()
        {
            static uint index = 0;
            id = ClientId(index++);
        }

        ClientData(const ClientData&) = delete;
        ClientData(const ClientData&&) = delete;

};

class IServerImplHook;
class FLHook;
class ClientList
{
        friend FLHook;
        friend IServerImplHook;

        std::array<ClientData, MaxClientId + 1> clients;

        void PlayerConnect(uint clientId)
        {
            clients[clientId].isValid = true;
            if (clientId > largestClientId)
            {
                largestClientId = clientId;
            }
            if (clientId < smallestClientId)
            {
                smallestClientId = clientId;
            }
        }

        void PlayerDisconnect(uint clientId)
        {
            clients[clientId].isValid = false;
            if (largestClientId == clientId)
            {
                for (uint i = largestClientId - 1; i != smallestClientId; i--)
                {
                    if (clients[i].isValid == true)
                    {
                        largestClientId = i;
                        break;
                    }
                }
            }
            if (smallestClientId == clientId)
            {
                for (uint i = smallestClientId + 1; i != smallestClientId; i++)
                {
                    if (clients[i].isValid == true)
                    {
                        smallestClientId = i;
                        break;
                    }
                }
            }
        }

        inline static uint largestClientId = 0;
        inline static uint smallestClientId = 0;

        ClientList() = default; // Private constructor so it can only be constructed by FLHook

    public:
        class Iterator
        {
                friend ClientList;
                ClientData* data;
                ClientList* list;

                using Tr = std::iterator_traits<ClientData*>;
                explicit Iterator(ClientData* clientData, ClientList* list) : data(clientData), list(list) {}

            public:
                using Reference = Tr::reference;
                using Pointer = Tr::pointer;

                Iterator& operator++()
                {
                    const auto end = list->clients.data() + list->clients.size();
                    while (++data < end && data->id.GetValue() < largestClientId && !data->isValid) {}

                    return *this;
                }

                Pointer operator->() const { return data; }
                Reference operator*() const { return *data; }

                friend bool operator==(const Iterator& a, const Iterator& b) { return a.data == b.data; }
                friend bool operator!=(const Iterator& a, const Iterator& b) { return a.data != b.data; }
        };

        ClientData& operator[](const size_t index)
        {
            if (index > MaxClientId)
            {
                throw std::out_of_range("Attempting to access client id that is out of range is invalid.");
            }

            return clients[index];
        }

        ClientData& operator[](const ClientId client) { return this->operator[](client.GetValue()); }

        Iterator begin() { return Iterator(&clients[smallestClientId], this); }

        Iterator end()
        {
            auto* client = &clients[largestClientId];
            client++;
            return Iterator(client, this);
        }
};
