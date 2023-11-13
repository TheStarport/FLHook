#pragma once

#include "RepGroupId.hpp"

class RepId final
{
        int value = 0;

    public:
        explicit RepId(const ObjectId& spaceObj, bool isSolar);
        explicit RepId(const int val) : value(val) {}
        explicit RepId() = default;
        ~RepId() = default;
        RepId(const RepId&) = default;
        RepId& operator=(RepId) = delete;
        RepId(RepId&&) = default;
        RepId& operator=(RepId&&) = delete;

        bool operator==(const RepId& next) const { return value == next.value; }
        explicit operator bool() const;

        int GetValue() const { return value; }

        Action<RepGroupId, Error> GetAffiliation() const;
        Action<float, Error> GetAttitudeTowardsRepId(const RepId& target) const;
        Action<float, Error> GetAttitudeTowardsFaction(const RepGroupId& group) const;
        Action<int, Error> GetRank() const;
        Action<std::pair<FmtStr, FmtStr>, Error> GetName() const;

        Action<void, Error> SetRank(int rank) const;
        Action<void, Error> SetAttitudeTowardsRepId(RepId target, float newAttitude) const;
        Action<void, Error> SetAffiliation(RepGroupId group) const;
};
