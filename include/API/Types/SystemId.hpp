#pragma once

class SystemId
{
        const uint value;

    public:
        explicit SystemId(const uint val) : value(val) {}
        explicit operator uint() const noexcept { return value; }
        bool operator==(const SystemId next) const { return value == next.value; }
        bool operator!() const; // TODO: Check if systemId is valid

        std::wstring GetName();
        std::wstring GetNickName();
        std::vector<Zone&> GetZones();
        std::vector<SystemId> GetNeighboringSystems(); //TODO: Look into Freelancer System Enumerator.
        std::vector<CSolar*> GetSolars(bool onlyDockables = false);
        std::vector<ClientId> GetPlayersInSystem();
        std::vector<ShipId> GetShipsInSystem();

        void SendMessageToSystem(std::wstring_view, std::optional<MessageFormat> format = {}, std::optional<MessageColor> color = {});
        void SetSystemMusic(std::wstring trackNickName,std::optional<std::pair<Vector, float>> sphere = {});
        void PlaySound(std::wstring trackNickNameSound, std::optional<std::pair<Vector, float>> sphere = {});
        void ToggleSystemLock(bool locked); //TODO: Check into finding ways to lock gates.
        void ToggleDockables(bool locked);
        uint KillAllPlayers();
};