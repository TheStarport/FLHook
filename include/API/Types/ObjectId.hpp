#pragma once

#include "RepId.hpp"
#include "SystemId.hpp"

/**
 * @brief This object represents instances of both Stations and Ships in space.
 */
class DLL ObjectId
{
    protected:
        uint value = 0;

    public:
        explicit ObjectId(const uint val) : value(val) {}
        explicit ObjectId() = default;

        explicit operator uint() const noexcept { return value; }
        bool operator==(const ObjectId& next) const { return value == next.value; }
        explicit operator bool() const;

        uint GetValue() const { return value; }

        [[nodiscard]]
        Action<CObject::Class, Error> GetObjectType() const;

        [[nodiscard]]
        Action<std::wstring, Error> GetNickName() const;

        [[nodiscard]]
        Action<CSimplePtr, Error> GetCObject(bool increment = false) const;

        [[nodiscard]]
        Action<Archetype::Root*, Error> GetArchetype() const;

        [[nodiscard]]
        Action<std::pair<Vector, float>, Error> GetVelocityAndSpeed() const;

        [[nodiscard]]
        Action<Vector, Error> GetAngularVelocity() const;

        [[nodiscard]]
        Action<std::pair<Vector, Matrix>, Error> GetPositionAndOrientation() const;

        [[nodiscard]]
        Action<SystemId, Error> GetSystem() const;

        [[nodiscard]]
        Action<RepId, Error> GetReputation() const;
};

template <>
struct std::formatter<ObjectId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const ObjectId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};
