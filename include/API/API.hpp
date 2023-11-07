#pragma once

class AdminApi;
class ClientApi;
class MathApi;
class PersonalitiesApi;
class PlayerApi;
class SolarApi;
class ZoneApi;

class DLL Api
{
        friend AdminApi;
        friend ClientApi;
        friend MathApi;
        friend PersonalitiesApi;
        friend PlayerApi;
        friend SolarApi;
        friend ZoneApi;

        static ClientId ExtractClientID(const std::variant<uint, std::wstring_view>& player)
        {
            // If index is 0, we just use the client Id we are given
            if (!player.index())
            {
                const uint id = std::get<uint>(player);
                return IsValidClientId(id) ? id : -1;
            }

            // Otherwise we have a character name
            const std::wstring_view characterName = std::get<std::wstring_view>(player);

            const auto client = GetClientIdFromCharName(characterName).Raw();
            if (client.has_error())
            {
                return UINT_MAX;
            }

            return client.value();
        }

    public:
        /**
         * Gets the current client id of the account.
         * @param acc CAccount
         * @returns On success : Client id of the active user of the account.
         * @returns On fail : [PlayerNotLoggedIn] The function could not find a client id associated with the account id.
         */
        static Action<uint, Error> GetClientIdFromAccount(const CAccount* acc);

        static std::wstring GetAccountIdByClientID(ClientId client);

        static CAccount* GetAccountByClientID(ClientId client);

        /**
         * Gets the current client id of the character.
         * @param character Wide string of the character name
         * @returns On success : current client Id associated with that character name
         * @returns On fail : [CharacterDoesNotExist] The function could not find a client id associated with this character name.
         */
        static Action<uint, Error> GetClientIdFromCharName(std::wstring_view character);

        /**
         * Gets the account of the character
         * @param character Wide string of the character name
         * @returns On success : the account Id for that character
         * @returns On fail : [CharacterDoesNotExist] The function could not find the account id associated with this character name.
         */
        static Action<CAccount*, Error> GetAccountByCharName(std::wstring_view character);

        /**
         * Gets the account id in a wide string
         * @param acc The account
         * @returns On success : wide string of account Id
         * @returns On fail : [CannotGetAccount] The function could not find the account.
         */
        static Action<std::wstring, Error> GetAccountID(CAccount* acc);

        static Action<std::wstring, Error> GetCharacterNameByID(ClientId client);

        /**
         * Checks to see if the client Id is valid
         * @param id Client Id
         * @returns If Valid: true
         * @returns If Not Valid: false
         */
        static bool IsValidClientId(ClientId id);

        static bool IsInCharSelectMenu(const uint& player);

        static Action<ClientId, Error> GetClientIdByShip(ShipId ship);

        static void PrintUserCmdText(ClientId client, std::wstring_view text);
        static void PrintLocalUserCmdText(ClientId client, std::wstring_view msg, float distance);
};

#include "API/FLServer/Admin.hpp"
#include "API/FLServer/Chat.hpp"
#include "API/FLServer/Client.hpp"
#include "API/FLServer/Math.hpp"
#include "API/FLServer/Personalities.hpp"
#include "API/FLServer/Player.hpp"
#include "API/FLServer/Solar.hpp"
#include "API/FLServer/ZoneUtilities.hpp"

#include "API/FLHook/ClientInfo.hpp"
#include "API/FLHook/MailManager.hpp"
#include "API/FLHook/Plugin.hpp"

#include "API/Utils/FileUtils.hpp"
#include "API/Utils/IniUtils.hpp"
#include "API/Utils/PerfTimer.hpp"
