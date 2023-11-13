#pragma once

class ClientId;
class BaseId;
class ShipId;

class SystemId
{
        uint value;

    public:
        explicit SystemId(const uint val) : value(val) {}
        explicit SystemId() : value(0) {}
        bool operator==(const SystemId next) const { return value == next.value; }
        explicit operator bool() const;

        Action<std::wstring_view, Error> GetName() const;
        Action<std::wstring, Error> GetNickName() const;
        Action<std::vector<Universe::IZone*>, Error> GetZones() const;
        Action<std::vector<SystemId>, Error> GetNeighboringSystems() const; // TODO: Look into Freelancer System Enumerator.
        Action<std::vector<CSolar*>, Error> GetSolars(bool onlyDockables = false);
        Action<std::vector<ClientId>, Error> GetPlayersInSystem(bool includeDocked = false) const;
        Action<std::vector<ShipId>, Error> GetShipsInSystem();

        Action<void, Error> Message(std::wstring_view, std::optional<MessageFormat> format = {}, std::optional<MessageColor> color = {});
        Action<void, Error> SetSystemMusic(std::wstring trackNickName, std::optional<std::pair<Vector, float>> sphere = {});
        Action<void, Error> PlaySound(std::wstring trackNickNameSound, std::optional<std::pair<Vector, float>> sphere = {});
        Action<void, Error> ToggleSystemLock(bool locked); // TODO: Check into finding ways to lock gates.
        Action<void, Error> ToggleDockables(bool locked);
        Action<uint, Error> KillAllPlayers() const;
};
