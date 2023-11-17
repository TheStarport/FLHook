#include "PCH.hpp"

#include "API/FLHook/InfocardManager.hpp"
#include "API/Types/SystemId.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"
#include "Core/FLHook.hpp"

#define ValidSystemCheck                            \
    if (!this)                                      \
    {                                               \
        return { cpp::fail(Error::InvalidSystem) }; \
    }

SystemId::SystemId(std::wstring_view nickName, bool isInfoCardName)
{
    if (!isInfoCardName)
    {
        value = CreateID(StringUtils::wstos(std::wstring(nickName)).c_str());
    }
    // TODO: Construct systemID from the infocard name.
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

Action<std::wstring, Error> SystemId::PositionToSectorCoord(const Vector& pos) const
{
    const Universe::ISystem* system = Universe::get_system(value);
    if (!system)
    {
        return { cpp::fail(Error::InvalidSystem) };
    }

    const float scale = system->navMapScale;

    const float gridSize = 34000.0f / scale;
    int gridRefX = static_cast<int>((pos.x + gridSize * 5) / gridSize) - 1;
    int gridRefZ = static_cast<int>((pos.z + gridSize * 5) / gridSize) - 1;

    gridRefX = std::min(std::max(gridRefX, 0), 7);
    char xPos = 'A' + static_cast<char>(gridRefX); // NOLINT

    gridRefZ = std::min(std::max(gridRefZ, 0), 7);
    char zPos = '1' + static_cast<char>(gridRefZ); // NOLINT

    return { std::format(L"{}-{}", xPos, zPos) };
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

Action<std::vector<CSolar*>, Error> SystemId::GetSolars(bool onlyDockables)
{
    // TODO: Implement
    return { {} };
}

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

Action<void, Error> SystemId::Message(std::wstring_view msg, MessageColor color, MessageFormat format)
{
    ValidSystemCheck;

    const auto clientsInSystem = GetPlayersInSystem().Raw();

    if (clientsInSystem.has_error())
    {
        return { cpp::fail(clientsInSystem.error()) };
    }

    Error err = Error::Default;

    for (auto& client : clientsInSystem.value())
    {
        const auto res = client.Message(msg, format, color).Raw();
        if (res.has_error() && err != Error::Default)
        {
            err = res.error();
        }
    }
    if (err != Error::Default)
    {
        return { cpp::fail(err) };
    }
    return { {} };
}

Action<void, Error> SystemId::PlaySoundOrMusic(const std::wstring& trackNickNameSound, bool isMusic, const std::optional<std::pair<Vector, float>>& sphere)
{
    ValidSystemCheck;
    auto sound = pub::Audio::Tryptich();
    uint id = InternalApi::CreateID(trackNickNameSound);
    sound.musicId = id;
    const auto clientsInSystem = GetPlayersInSystem().Raw();

    if (clientsInSystem.has_error())
    {
        return { cpp::fail(clientsInSystem.error()) };
    }

    if (!sphere.has_value())
    {
        for (const auto& client : clientsInSystem.value())
        {
            if (isMusic)
            {
                if (pub::Audio::SetMusic(client.GetValue(), sound) != (int)ResponseCode::Success)
                {
                    return { cpp::fail(Error::InvalidSoundId) };
                }
            }
            else if (pub::Audio::PlaySoundEffect(client.GetValue(), id) != (int)ResponseCode::Success)
            {
                return { cpp::fail(Error::InvalidSoundId) };
            }
        }
        return { {} };
    }

    for (const auto& client : clientsInSystem.value())
    {
        const auto clientPos = client.GetShipId().Handle().GetPositionAndOrientation().Handle().first;

        if (glm::length<3, float, glm::packed_highp>(clientPos - sphere.value().first) < sphere.value().second)
        {
            if (isMusic)
            {
                if (pub::Audio::SetMusic(client.GetValue(), sound) != (int)ResponseCode::Success)
                {
                    return { cpp::fail(Error::InvalidSoundId) };
                }

                if (pub::Audio::PlaySoundEffect(client.GetValue(), id) != (int)ResponseCode::Success)
                {
                    return { cpp::fail(Error::InvalidSoundId) };
                }
            }
        }
    }

    return { {} };
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
