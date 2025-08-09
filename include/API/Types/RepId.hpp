#pragma once

#include "RepGroupId.hpp"

#include <FLCore/Common/FmtStr.hpp>

class ObjectId;

class DLL RepId final
{
        int value = 0;

    public:
        explicit RepId(const int val) : value(val) {}
        explicit RepId() = default;
        ~RepId() = default;

        explicit operator int() const noexcept { return value; }
        bool operator==(const RepId& next) const { return value == next.value; }
        explicit operator bool() const;

        [[nodiscard]]
        int GetValue() const;

        [[nodiscard]]
        Action<RepGroupId> GetAffiliation() const;
        [[nodiscard]]
        Action<float> GetAttitudeTowardsRepId(const RepId& target) const;
        [[nodiscard]]
        Action<float> GetAttitudeTowardsFaction(const RepGroupId& group) const;
        [[nodiscard]]
        Action<int> GetRank() const;
        [[nodiscard]]
        Action<std::pair<FmtStr, FmtStr>> GetName() const;

        [[nodiscard]]
        Action<void> SetRank(int rank) const;
        [[nodiscard]]
        Action<void> SetAttitudeTowardsRepId(RepId target, float newAttitude) const;
        [[nodiscard]]
        Action<void> SetAttitudeTowardsRepGroupId(RepGroupId target, float newAttitude) const;
        [[nodiscard]]
        Action<void> SetAffiliation(RepGroupId group) const;
};

template <>
struct std::formatter<RepId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const RepId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};
