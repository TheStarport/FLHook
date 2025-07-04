#pragma once

#include "Mail.hpp"

/**
 * @brief A class for sending messages and information to characters or accounts regardless of online status.
 * @note If using this class you should not be in the main FLHook thread. These are database calls and will be blocking.
 */
class MailManager
{
        static concurrencpp::result<void>InformOnlineUsersOfNewMail(std::vector<rfl::Variant<bsoncxx::oid, std::string>> accountIdOrCharacterNames);
    public:
        MailManager() = delete;
        static Action<std::vector<Mail>> GetAccountMail(std::string accountId, int count = 20, int page = 1, bool newestFirst = true);
        static Action<std::vector<Mail>> GetCharacterMail(bsoncxx::oid characterId, int count = 20, int page = 1, bool newestFirst = true);
        static Action<void> DeleteMail(const Mail& mail);
        static Action<void> MarkMailAsRead(const Mail& mail, rfl::Variant<std::string, bsoncxx::oid> characterOrAccount);
        static Action<void> SendMail(Mail& mail);
};
