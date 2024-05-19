#pragma once

#include "Mail.hpp"

class MailManager
{
        inline static std::mutex mailCallbackMutex;
        inline static int inflightMailId = 0;
        inline static std::unordered_map<int, std::vector<std::wstring>> inflightMail;
        static void SendMailCallback(int id);
        static void GetMailCallback(std::shared_ptr<std::vector<Mail>> data, std::function<void(std::vector<Mail>)> callback);
        static void ParseMail(Mail& mail, bsoncxx::document::view doc);

    public:
        static void GetAccountMail(const AccountId& id, std::function<void(std::vector<Mail>)> callback, int count = 20, int page = 1, bool newestFirst = true);
        static void GetCharacterMail(bsoncxx::oid characterId, std::function<void(std::vector<Mail>)> callback, int count = 20, int page = 1, bool newestFirst = true);
        static void MarkMailAsRead(const Mail& mail, bsoncxx::oid character);
        static Action<void, Error> SendMail(const Mail& mail);
};
