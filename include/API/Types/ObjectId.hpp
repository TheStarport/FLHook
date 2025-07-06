#pragma once

#include "RepId.hpp"
#include "SystemId.hpp"

class CSimple;
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

        /**
         * @param preventDamage If true prevent all damage to this entity
         * @param allowPlayerDamage If true, and the previous false, prevent damage from all non-player sources
         * @param maxHpLossPercentage A value between 0 and 1. Will stop health from falling below the specified value
         */
        Action<void> SetInvincible(bool preventDamage, bool allowPlayerDamage, float maxHpLossPercentage = 0.0f) const;
};

template <>
struct std::formatter<ObjectId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const ObjectId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetId().Unwrap()); }
};
