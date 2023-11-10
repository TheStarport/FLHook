#pragma once

#include "SystemId.hpp"

class ObjectId
{
    protected:
        const uint value = 0;

    public:
        explicit ObjectId(const uint val) : value(val) {}
        explicit ObjectId() = default;
        virtual ~ObjectId() = default;
        ObjectId(const ObjectId&) = default;
        ObjectId& operator=(ObjectId) = default;
        ObjectId(ObjectId&&) = default;
        ObjectId& operator=(ObjectId&&) = delete;

        bool operator==(const ObjectId& next) const { return value == next.value; }
        explicit virtual operator bool() const;

        [[nodiscard]]
        Action<CObject::Class, Error> GetObjectType() const;

        [[nodiscard]]
        Action<std::wstring, Error> GetNickName() const;

        [[nodiscard]]
        Action<std::wstring, Error> GetName() const;

        [[nodiscard]]
        Action<CObjPtr, Error> GetCObject(bool increment = false) const;

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
};
