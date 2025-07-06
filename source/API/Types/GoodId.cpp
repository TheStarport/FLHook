#include "PCH.hpp"

#include <API/FLHook/InfocardManager.hpp>
#include <API/Types/GoodId.hpp>

GoodId::GoodId(const uint hash) : value(const_cast<GoodInfo*>(GoodList::find_by_id(hash))) {}
GoodId::GoodId(GoodInfo* good) : value(good){}
bool GoodId::operator==(const GoodId& next) const { return value == next.value; }

// TODO: Handle error code!
#define GoodCheck                                  \
    if (!value)                                    \
    {                                              \
        return { cpp::fail(Error::InvalidGood) }; \
    }

GoodId::operator bool() const { return value; }

const GoodInfo* GoodId::GetValue() const { return value; }

Action<Id> GoodId::GetHash() const
{
    GoodCheck;

    return { Id(value->goodId) };
}

Action<EquipmentId> GoodId::GetEquipment() const
{
    GoodCheck;

    if (value->type == GoodType::Hull)
    {
        return { EquipmentId(value->shipGoodId) };
    }

    return { EquipmentId(value->equipmentId) };
}

Action<std::wstring_view> GoodId::GetName() const
{
    GoodCheck;

    return { FLHook::GetInfocardManager()->GetInfoName(value->idsName) };
}

Action<float> GoodId::GetPrice() const
{
    GoodCheck;

    return { value->price };
}

Action<GoodType> GoodId::GetType() const
{
    GoodCheck;

    return { value->type };
}

bool GoodId::IsCommodity() const
{
    if (!value)
    {
        return false;
    }

    return value->type == GoodType::Commodity;
}
