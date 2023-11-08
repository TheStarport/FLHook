#include "PCH.hpp"

#include "core/ApiInternal.hpp"



ClientId ExtractClientID(const std::variant<uint, std::wstring_view>& player)
{
    // If index is 0, we just use the client Id we are given
    if (!player.index())
    {
        const uint id = std::get<uint>(player);
        return HkApi::IsValidClientId(id) ? id : -1;
    }

    // Otherwise we have a character name
    const std::wstring_view characterName = std::get<std::wstring_view>(player);

    const auto client = HkApi::GetClientIdFromCharName(characterName).Raw();
    if (client.has_error())
    {
        return UINT_MAX;
    }

    return client.value();
}
