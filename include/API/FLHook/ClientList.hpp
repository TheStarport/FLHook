#pragma once

struct ClientData
{
        ClientId id;
        CAccount* account;
        std::wstring_view name;
        ClientInfo info;
        PlayerData* playerData;
        bool online;
};

class ClientList
{
        friend FLHook;

        inline static std::array<ClientData, MaxClientId> clients;

        void PlayerConnect(uint ClientId);
        void PlayerDisconnect(uint ClientId);
        void PlayerCharacterSelect();
        void PlayerLogout();


        uint largestClientId = 0;
        uint smallestClientId = 0;

    public:
        ClientList();

        struct Iterator
        {
                explicit Iterator(ClientData* clientData);

                Iterator operator++()
                {
                    index++;
                    data = &clients[index];
                    while (!data->online)
                    {
                        index++;
                        data = &clients[index];
                    }
                    return Iterator(data);
                }
                Iterator operator=()
                {


                }
                Iterator operator*() {}

            private:
                uint index;
                ClientData* data;
        };
        std::array<ClientData, MaxClientId>& GetClientList();

        ClientData& operator[]();
        Iterator begin()
        {
            auto& client = clients[smallestClientId];
            return Iterator(&client);
        }
        Iterator end()
        {
            auto& client = clients[largestClientId];
            return Iterator(&client);
        }
};
