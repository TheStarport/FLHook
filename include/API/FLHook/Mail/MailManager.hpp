#pragma once

#include "Mail.hpp"

class MailManager
{
        static void SendMailCallback(std::any taskData);
        static bool DeferSendMail(std::any taskData, Mail mail);
        static bool GetMailForCharacter(std::any taskData, bsoncxx::oid characterId, int count, int page, bool newestFirst);
        static bool GetMailForAccount(std::any taskData, AccountId accountId, int count, int page, bool newestFirst);
        static void ParseMail(Mail& mail, bsoncxx::document::view doc);

    public:
        static void GetAccountMail(const AccountId& id, std::function<void(std::vector<Mail>*)> callback, int count = 20, int page = 1,
                                   bool newestFirst = true);
        static void GetCharacterMail(bsoncxx::oid characterId, std::function<void(std::vector<Mail>*)> callback, int count = 20, int page = 1,
                                     bool newestFirst = true);
        static void MarkMailAsRead(const Mail& mail, bsoncxx::oid character);
        static Action<void, Error> SendMail(const Mail& mail);
};
