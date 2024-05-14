#pragma once

#include "Mail.hpp"

class MailManager
{
        inline static std::mutex mailCallbackMutex;
        inline static int inflightMailId = 0;
        inline static std::unordered_map<int, std::vector<std::wstring>> inflightMail;
        static void SendMailCallback(int id);

    public:
        static void GetAccountMail(const AccountId& id, std::function<void(std::vector<Mail>)> callback, int count = 20, int page = 1, bool newestFirst = true);
        static void GetCharacterMail(bsoncxx::oid characterId, std::function<void(std::optional<Mail>)> callback, int count = 20, int page = 1, bool newestFirst = true);
        static Action<void, Error> SendMail(const Mail& mail);
};
