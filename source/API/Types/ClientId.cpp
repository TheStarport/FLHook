#include "PCH.hpp"

#include "API/API.hpp"
#include "API/Types/ClientId.hpp"

bool ClientId::IsValidClientId() { return (value > 0 && value < 256); }

Action<std::wstring, Error> ClientId::GetCharacterName()
{
    if (!IsValidClientId())
    {
        return { cpp::fail(Error::InvalidClientId) };
    }
    return { reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(value)) };
}

Action<BaseId,Error> ClientId::GetCurrentBase()
{


}