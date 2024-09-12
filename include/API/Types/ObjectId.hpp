#pragma once

#include "FLCore/Common/CObjects/CSimple.hpp"
#include "RepId.hpp"
#include "SystemId.hpp"

/**
 * @brief This object represents instances of both Stations and Ships in space.
 */
class DLL ObjectId
{
    protected:
        std::weak_ptr<CSimple> value{};

    public:
        explicit ObjectId(uint val);
        explicit ObjectId() = default;

        bool operator==(const ObjectId& next) const;
        explicit operator bool() const;

        [[nodiscard]]
        std::weak_ptr<CSimple> GetValue() const;

        [[nodiscard]]
        Action<uint> GetId() const;

        [[nodiscard]]
        Action<CObject::Class> GetObjectType() const;

        [[nodiscard]]
        Action<std::wstring> GetNickName() const;

        [[nodiscard]]
        Action<Archetype::Root*> GetArchetype() const;

        [[nodiscard]]
        Action<std::pair<Vector, float>> GetVelocityAndSpeed() const;

        [[nodiscard]]
        Action<Vector> GetAngularVelocity() const;

        [[nodiscard]]
        Action<std::pair<Vector, Matrix>> GetPositionAndOrientation() const;

        [[nodiscard]]
        Action<SystemId> GetSystem() const;

        [[nodiscard]]
        Action<RepId> GetReputation() const;

        [[nodiscard]]
        Action<float> GetHealth(bool percentage = false) const;

        [[nodiscard]]
        Action<ClientId> GetPlayer() const;
};

template <>
struct std::formatter<ObjectId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const ObjectId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetId().Unwrap()); }
};
