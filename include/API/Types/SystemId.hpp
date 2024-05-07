#pragma once

class ClientId;
class BaseId;
class ShipId;

class DLL SystemId
{
        uint value;

    public:
        explicit SystemId(const uint val) : value(val) {}

        explicit SystemId(std::wstring_view nickName, bool isInfoCardName = false);

        SystemId() : value(0) {}
        bool operator==(const SystemId next) const { return value == next.value; }
        explicit operator bool() const;

        uint GetValue() const { return value; }

        Action<std::wstring_view, Error> GetName() const;
        Action<std::wstring, Error> GetNickName() const;
        Action<std::vector<Universe::IZone *>, Error> GetZones() const;
        Action<std::wstring, Error> PositionToSectorCoord(const Vector &pos) const;
        Action<std::vector<SystemId>, Error> GetNeighboringSystems() const; // TODO: Look into Freelancer System Enumerator.
        Action<std::vector<CSolar *>, Error> GetSolars(bool onlyDockables = false);
        Action<std::vector<ClientId>, Error> GetPlayersInSystem(bool includeDocked = false) const;

        Action<void, Error> Message(std::wstring_view msg, MessageColor color = MessageColor::Default, MessageFormat format = MessageFormat::Normal) const;
        Action<void, Error> PlaySoundOrMusic(const std::wstring &trackNickNameSound, bool isMusic = false,
                                             const std::optional<std::pair<Vector, float>> &sphere = {}) const;
        Action<uint, Error> KillAllPlayers() const;
};

template <>
struct std::formatter<SystemId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context &ctx) const { return ctx.begin(); }
        auto format(const SystemId &value, std::wformat_context &ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};
