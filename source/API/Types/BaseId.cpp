#include "PCH.hpp"

#include "API/Types/BaseId.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/InfocardManager.hpp"

#define ValidBaseCheck                            \
    if (!this->operator bool())                   \
    {                                             \
        return { cpp::fail(Error::InvalidBase) }; \
    }

BaseId::BaseId(const std::wstring_view baseName, const bool isWildCard)
{
    const std::string str = StringUtils::wstos(std::wstring(baseName));
    auto base = Universe::get_base(CreateID(str.c_str()));

    if (base)
    {
        value = base->baseId;
        return;
    }

    const auto& im = FLHook::GetInfocardManager();
    base = Universe::GetFirstBase();
    do
    {
        if(isWildCard)
        {
            if (auto name = im->GetInfoName(base->baseIdS); wildcards::match(name, baseName))
            {
                value = base->baseId;
                return;
            }
        }
        else
        {
            if (auto name = im->GetInfoName(base->baseIdS); name == baseName)
            {
                value = base->baseId;
                return;
            }
        }


        base = Universe::GetNextBase();
    }
    while (base);

    value = 0;
}

BaseId::operator bool() const { return Universe::get_base(value) != nullptr; }

Action<ObjectId> BaseId::GetSpaceId() const
{
    ValidBaseCheck;

    const auto base = Universe::get_base(value);
    if (!this->operator bool())
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    return { ObjectId(base->spaceObjId) };
}

Action<SystemId> BaseId::GetSystem() const
{
    Universe::IBase* ibase = Universe::get_base(value);
    return { SystemId(ibase->systemId) };
}

Action<RepId> BaseId::GetAffiliation() const
{
    // Technically the base rep is defined in mbases, but we don't have access to it at the moment
    // this method matches every base in vanilla, so we go with this instead.
    const auto objectId = GetSpaceId().Unwrap();
    if (!objectId)
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    return objectId.GetReputation();
}

Action<std::wstring_view> BaseId::GetName() const
{
    const auto base = Universe::get_base(value);
    if (!base)
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    return { FLHook::GetInfocardManager()->GetInfoName(base->baseIdS) };
}

Action<std::pair<float, float>> BaseId::GetBaseHealth() const
{
    float curHealth;
    float maxHealth;
    const Universe::IBase* base = Universe::get_base(value);

    if (!base)
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    pub::SpaceObj::GetHealth(base->spaceObjId, curHealth, maxHealth);
    return { std::make_pair(curHealth, maxHealth) };
}

Action<std::pair<std::wstring_view, std::wstring_view>> BaseId::GetDescription() const
{
    ValidBaseCheck;

    // TODO: Get internal description of base
    return { {} };
}

Action<std::vector<uint>> BaseId::GetItemsForSale() const
{
    ValidBaseCheck;

    std::array<byte, 2> nop = { 0x90, 0x90 };
    std::array<byte, 2> jnz = { 0x75, 0x1D };
    MemUtils::WriteProcMem(FLHook::Offset(FLHook::BinaryType::Server, AddressList::GetCommodities), nop.data(), 2); // patch, else we only get commodities

    static std::array<uint, 1024> arr;
    std::fill_n(arr.begin(), arr.size(), 0);

    int size = 256;
    pub::Market::GetCommoditiesForSale(value, arr.data(), &size);
    MemUtils::WriteProcMem(FLHook::Offset(FLHook::BinaryType::Server, AddressList::GetCommodities), jnz.data(), 2);

    return { std::vector(arr.begin(), arr.begin() + size) };
}

Action<float> BaseId::GetCommodityPrice(GoodId goodId) const
{
    float nomPrice;
    if (pub::Market::GetNominalPrice(goodId.GetHash().Unwrap(), nomPrice) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidGood) };
    }

    float price;
    if (pub::Market::GetPrice(value, goodId.GetHash().Unwrap(), price) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    return { price };
}

Action<std::vector<ClientId>> BaseId::GetDockedPlayers() const
{
    ValidBaseCheck;

    std::vector<ClientId> players;

    for (auto& client : FLHook::Clients())
    {
        if (client.baseId == *this)
        {
            players.emplace_back(client.id);
        }
    }

    return { players };
}
