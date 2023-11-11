#include "PCH.hpp"

#include "API/Types/ObjectId.hpp"

ObjectId::operator bool() const { return pub::SpaceObj::ExistsAndAlive(value) == static_cast<int>(ResponseCode::Success); }

Action<CObject::Class, Error> ObjectId::GetObjectType() const
{
    uint type;
    if (pub::SpaceObj::GetType(value, type) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { static_cast<CObject::Class>(type) };
}

Action<std::wstring, Error> ObjectId::GetNickName() const
{
    auto obj = GetCObject();
    if (obj.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { StringUtils::stows(obj.Raw().value()->get_archetype()->name) };
}

Action<std::wstring, Error> ObjectId::GetName() const
{
    // TODO: Get Ids name as string
    return { {} };
}

Action<CObjPtr, Error> ObjectId::GetCObject(bool increment) const
{
    auto obj = CObjPtr(value);
    if (obj)
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { CObjPtr(obj.get(), increment) };
}

Action<Archetype::Root*, Error> ObjectId::GetArchetype() const
{
    auto obj = GetCObject();
    if (obj.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { obj.Raw().value()->get_archetype() };
}
Action<std::pair<Vector, float>, Error> ObjectId::GetVelocityAndSpeed() const
{
    Vector velocity, angularVelocity;
    if (pub::SpaceObj::GetMotion(value, velocity, angularVelocity) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { std::make_pair(velocity, glm::length<3, float, glm::highp>(velocity)) };
}

Action<Vector, Error> ObjectId::GetAngularVelocity() const
{
    Vector velocity, angularVelocity;
    if (pub::SpaceObj::GetMotion(value, velocity, angularVelocity) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { angularVelocity };
}

Action<std::pair<Vector, Matrix>, Error> ObjectId::GetPositionAndOrientation() const
{
    Vector pos;
    Matrix rot;
    if (pub::SpaceObj::GetLocation(value, pos, rot) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { std::make_pair(pos, rot) };
}

Action<SystemId, Error> ObjectId::GetSystem() const
{
    uint system;
    pub::SpaceObj::GetSystem(value, system);

    if (!system)
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { SystemId(system) };
}
