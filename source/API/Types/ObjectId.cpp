#include "PCH.hpp"

#include "API/Types/ObjectId.hpp"

ObjectId::operator bool() const { return GetCObject().Raw().has_value(); }

CObject::Class ObjectId::GetObjectType() const { return GetCObject().Unwrap()->objectClass; }

std::wstring ObjectId::GetNickName() const { return StringUtils::stows(GetCObject().Unwrap()->get_archetype()->name); }

std::wstring ObjectId::GetName() const
{
    // TODO: Get Ids name as string
    return {};
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

Archetype::Root* ObjectId::GetArchetype() const { return GetCObject().Unwrap()->get_archetype(); }

Vector ObjectId::GetVelocity() const { return GetCObject().Unwrap()->get_velocity(); }

Vector ObjectId::GetAngularVelocity() const { return GetCObject().Unwrap()->get_angular_velocity(); }

Vector ObjectId::GetPosition() const { return GetCObject().Unwrap()->get_position(); }

Matrix ObjectId::GetOrientation() const { return GetCObject().Unwrap()->get_orientation(); }

SystemId ObjectId::GetSystem() const { return SystemId(GetCObject().Unwrap()->system); }
