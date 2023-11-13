#include "PCH.hpp"

#include "API/Types/SystemId.hpp"

#include "Core/FLHook.hpp"

#define ValidSystemCheck                            \
    if (!this)                                      \
    {                                               \
        return { cpp::fail(Error::InvalidSystem) }; \
    }

SystemId::operator bool() const { return Universe::get_system(value) != nullptr; }

Action<std::wstring_view, Error> SystemId::GetName() const
{
    ValidSystemCheck;
    uint ids;
    pub::System::GetName(value, ids);

    return { FLHook::GetInfocardManager().GetInfocard(ids) };
}

Action<std::wstring, Error> SystemId::GetNickName() const
{
    ValidSystemCheck;
    const auto system = Universe::get_system(value);
    return { StringUtils::stows(std::string(system->nickname)) };
}

Action<std::vector<Universe::IZone*>, Error> SystemId::GetZones() const
{
    ValidSystemCheck;

    const auto system = Universe::get_system(value);
    return { std::vector<Universe::IZone*>(system->zones.begin(), system->zones.end()) };
}

Action<std::vector<SystemId>, Error> SystemId::GetNeighboringSystems() const
{
    ValidSystemCheck;
    const auto system = Universe::get_system(value);

    std::vector<SystemId> vector(system->connections.size());
    for (uint i = 0u; i < vector.size(); ++i)
    {
        vector[i] = SystemId(system->connections[i]->id);
    }

    return { vector };
}

Action<std::vector<CSolar*>, Error> SystemId::GetSolars(bool onlyDockables) {}

Action<std::vector<ClientId>, Error> SystemId::GetPlayersInSystem(bool includeDocked) const
{
    ValidSystemCheck;

    std::vector<ClientId> clients;
    for (auto& client : FLHook::Clients())
    {
        if (!client.ship && !includeDocked)
        {
            continue;
        }

        uint systemId;
        pub::Player::GetSystem(client.id.GetValue(), systemId);

        if (systemId == value)
        {
            clients.emplace_back(client.id);
        }
    }

    return { clients };
}
Action<std::vector<ShipId>, Error> SystemId::GetShipsInSystem()
{
    //
    //
}

Action<void, Error> SystemId::Message(std::wstring_view, std::optional<MessageFormat> format, std::optional<MessageColor> color)
{
    //
    //
}
Action<void, Error> SystemId::SetSystemMusic(std::wstring trackNickName, std::optional<std::pair<Vector, float>> sphere)
{
    //
    //
}

Action<void, Error> SystemId::PlaySound(std::wstring trackNickNameSound, std::optional<std::pair<Vector, float>> sphere)
{
    //
    //
}

Action<void, Error> SystemId::ToggleSystemLock(bool locked)
{
    //
    //
}

Action<void, Error> SystemId::ToggleDockables(bool locked)
{
    //
    //
}

Action<uint, Error> SystemId::KillAllPlayers() const
{
    ValidSystemCheck;

    uint shipsKilled = 0;
    auto clients = GetPlayersInSystem().Unwrap();

    std::ranges::for_each(clients,
                          [&shipsKilled](const ClientId client)
                          {
                              client.GetData().ship.Destroy();
                              shipsKilled++;
                          });

    return { shipsKilled };
}
