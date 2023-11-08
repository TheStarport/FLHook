#pragma once

#include "API/FLServer/Admin.hpp"
#include "API/FLServer/Chat.hpp"
#include "API/FLServer/Client.hpp"
#include "API/FLServer/GameObject.hpp"
#include "API/FLServer/Math.hpp"
#include "API/FLServer/Personalities.hpp"
#include "API/FLServer/Player.hpp"
#include "API/FLServer/ZoneUtilities.hpp"

#include "API/FLHook/ClientInfo.hpp"
#include "API/FLHook/MailManager.hpp"
#include "API/FLHook/Plugin.hpp"

#include "API/Utils/FileUtils.hpp"
#include "API/Utils/IniUtils.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "API/Types/ClientId.hpp"
#include "API/Types/BaseId.hpp"
#include "Api/Types/ShipId.hpp"
#include "Api/Types/SystemId.hpp"


namespace HkApi
{
    // Making sure these functions is only accessible within Flhook

    /**
     * Gets the current client id of the account.
     * @param acc CAccount
     * @returns On success : Client id of the active user of the account.
     * @returns On fail : [PlayerNotLoggedIn] The function could not find a client id associated with the account id.
     */
    Action<uint, Error> GetClientIdFromAccount(const CAccount* acc);

    std::wstring GetAccountIdByClientID(ClientId client);

    CAccount* GetAccountByClientID(ClientId client);

    /**
     * Gets the current client id of the character.
     * @param character Wide string of the character name
     * @returns On success : current client Id associated with that character name
     * @returns On fail : [CharacterDoesNotExist] The function could not find a client id associated with this character name.
     */
    Action<uint, Error> GetClientIdFromCharName(std::wstring_view character);

    /**
     * Gets the account of the character
     * @param character Wide string of the character name
     * @returns On success : the account Id for that character
     * @returns On fail : [CharacterDoesNotExist] The function could not find the account id associated with this character name.
     */
    Action<CAccount*, Error> GetAccountByCharName(std::wstring_view character);

    /**
     * Gets the account id in a wide string
     * @param acc The account
     * @returns On success : wide string of account Id
     * @returns On fail : [CannotGetAccount] The function could not find the account.
     */
    DLL Action<std::wstring, Error> GetAccountID(CAccount* acc);

    DLL Action<std::wstring, Error> GetCharacterNameByID(ClientId client);

    /**
     * Checks to see if the client Id is valid
     * @param id Client Id
     * @returns If Valid: true
     * @returns If Not Valid: false
     */
    DLL bool IsValidClientId(ClientId id);

    DLL bool IsInCharSelectMenu(const uint& player);

    DLL Action<ClientId, Error> GetClientIdByShip(ShipId ship);

    DLL void PrintUserCmdText(ClientId client, std::wstring_view text);
    DLL void PrintLocalUserCmdText(ClientId client, std::wstring_view msg, float distance);

#ifdef FLHOOK


#endif

} // namespace HkApi