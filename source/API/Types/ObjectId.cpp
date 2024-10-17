#include "PCH.hpp"

#include "API/Types/ObjectId.hpp"

#include "API/FLHook/ResourceManager.hpp"

#define IsValidObj                                 \
    auto obj = value.lock();                       \
    if (!obj)                                      \
    {                                              \
        return { cpp::fail(Error::InvalidObject) }; \
    }

ObjectId::ObjectId(const uint val) { value = FLHook::GetResourceManager()->Get<CSimple>(val); }

bool ObjectId::operator==(const ObjectId& next) const
{
    const auto s1 = value.lock();
    const auto s2 = next.value.lock();

    return s1.get() == s2.get();
}
ObjectId::operator bool() const { return !value.expired(); }

std::weak_ptr<CSimple> ObjectId::GetValue() const { return value; }

Action<uint> ObjectId::GetId() const
{
    IsValidObj;

    return { obj->id };
}

Action<CObject::Class> ObjectId::GetObjectType() const
{
    IsValidObj;

    return { obj->objectClass };
}

Action<std::wstring> ObjectId::GetNickName() const
{
    IsValidObj;

    // TODO: Replace name which is actually the CMP with an ID look up (requires moving the create id detour from debug)
    return { StringUtils::stows(obj->get_archetype()->name) };
}

Action<Archetype::Root*> ObjectId::GetArchetype() const
{
    IsValidObj;

    return { obj->get_archetype() };
}
Action<std::pair<Vector, float>> ObjectId::GetVelocityAndSpeed() const
{
    IsValidObj;

    auto velocity = obj->get_velocity();
    return { std::make_pair(velocity, glm::length<3, float, glm::highp>(velocity)) };
}

Action<Vector> ObjectId::GetAngularVelocity() const
{
    IsValidObj;

    return { obj->get_angular_velocity() };
}

Action<std::pair<Vector, Matrix>> ObjectId::GetPositionAndOrientation() const
{
    IsValidObj;

    return { std::make_pair(obj->position, obj->orientation) };
}

Action<SystemId> ObjectId::GetSystem() const
{
    IsValidObj;

    return { SystemId(obj->system) };
}

Action<RepId> ObjectId::GetReputation() const
{
    IsValidObj;

    if (const auto* eq = dynamic_cast<CEqObj*>(obj.get()); eq)
    {
        return { RepId(eq->repVibe) };
    }

    return { cpp::fail(Error::InvalidRepInstance) };
}

// If provided a value of true in the argument it will provide a percentage value of the ship's health, with 0.0f being no hp and 1.0f being full health.
// Otherwise simply gives the health as an exact value.
// TODO: Move to header and doxify
Action<float> ObjectId::GetHealth(bool percentage) const
{
    IsValidObj;

    if (percentage)
    {
        return { obj->hitPoints / obj->archetype->hitPoints };
    }

    return { obj->hitPoints };
}

Action<ClientId> ObjectId::GetPlayer() const
{
    IsValidObj;

    if (auto ship = std::dynamic_pointer_cast<CShip>(obj); obj)
    {
        if (!ship->ownerPlayer)
        {
            return { cpp::fail(Error::ObjectIsNotAPlayer) };
        }

        return { ClientId(ship->ownerPlayer) };
    }

    return { cpp::fail(Error::ObjectIsNotAShip) };
}
