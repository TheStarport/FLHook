#include "PCH.hpp"

#include "API/API.hpp"
#include "Global.hpp"

using namespace sqlite_orm;

std::wstring_view MailManager::GetCharacterName(const std::variant<uint, std::wstring_view>& character)
{
    // If character is uint
    if (!character.index())
    {
        const auto name = std::get<uint>(character.GetCharacterName()).Raw();
        if (name.has_error())
        {
            return L""; // TODO: Get error
        }

        return name.value();
    }

    // Validate that the name is correct
    if (const auto acc = Hk::Client::GetAccountByCharName(std::get<std::wstring_view>(character)).Raw(); acc.has_error())
    {
        return L"";
    }

    return std::get<std::wstring_view>(character);
}

MailManager::MailManager() { storage.sync_schema(true); }

cpp::result<void, std::wstring> MailManager::SendNewMail(const std::variant<uint, std::wstring_view>& character, const MailItem& item)
{
    const auto characterName = GetCharacterName(character);
    if (characterName.empty())
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter)) };
    }

    if (item.subject.length() > 32)
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::SubjectTooLong)) };
    }

    if (item.author.length() > 36)
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::AuthorTooLong)) };
    }

    if (item.body.length() > 255)
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::BodyTooLong)) };
    }

    storage.insert(item);
    return {};
}

using namespace std::string_literals;

cpp::result<std::vector<MailManager::MailItem>, std::wstring> MailManager::GetMail(const std::variant<uint, std::wstring_view>& character,
                                                                                   const bool& ignoreRead, const int& page)
{
    const auto characterName = GetCharacterName(character);
    if (characterName.empty())
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter)) };
    }

    if (ignoreRead)
    {
        return storage.get_all<MailItem>(where(::c(&MailItem::unread) == true && ::c(&MailItem::characterName) == characterName),
                                         order_by(&MailItem::timestamp).desc(),
                                         limit(15, offset((page - 1) * 15)));
    }

    return storage.get_all<MailItem>(
        where(::c(&MailItem::characterName) == characterName), order_by(&MailItem::timestamp).desc(), limit(15, offset((page - 1) * 15)));
}

cpp::result<MailManager::MailItem, std::wstring> MailManager::GetMailById(const std::variant<uint, std::wstring_view>& character, const int64& index)
{
    const auto characterName = GetCharacterName(character);
    if (characterName.empty())
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter)) };
    }

    auto mail = storage.get<MailItem>(where(::c(&MailItem::characterName) == characterName && ::c(&MailItem::id) == index));

    mail.unread = false;
    storage.update_all(set(::c(&MailItem::unread) = mail.unread), where(::c(&MailItem::id) == mail.id));

    return mail;
}

cpp::result<void, std::wstring> MailManager::DeleteMail(const std::variant<uint, std::wstring_view>& character, const int64& index)
{
    const auto characterName = GetCharacterName(character);
    if (characterName.empty())
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter)) };
    }

    storage.remove<MailItem>(where(::c(&MailItem::characterName) == characterName && ::c(&MailItem::id) == index));

    return {};
}

cpp::result<int64, std::wstring> MailManager::PurgeAllMail(const std::variant<uint, std::wstring_view>& character, const bool& readMailOnly)
{
    const auto characterName = GetCharacterName(character);
    if (characterName.empty())
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter)) };
    }

    if (readMailOnly == true)
    {
        auto count = storage.count<MailItem>(where(::c(&MailItem::characterName) == characterName && ::c(&MailItem::unread) == false));
        storage.remove_all<MailItem>(where(::c(&MailItem::characterName) == characterName && ::c(&MailItem::unread) == false));
        return count;
    }
    auto count = storage.count<MailItem>(where(::c(&MailItem::characterName) == characterName));
    storage.remove_all<MailItem>(where(::c(&MailItem::characterName) == characterName));
    return count;
}

cpp::result<int64, std::wstring> MailManager::UpdateCharacterName(const std::wstring& oldCharacterName, const std::wstring& newCharacterName)
{
    if (const auto acc = Hk::Client::GetAccountByCharName(newCharacterName).Raw(); acc.has_error())
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter)) };
    }

    storage.update_all(set(::c(&MailItem::characterName) = newCharacterName), where(::c(&MailItem::characterName) == oldCharacterName));

    return storage.count<MailItem>(where(::c(&MailItem::characterName) == newCharacterName));
}

cpp::result<int64, std::wstring> MailManager::GetUnreadMailCount(const std::variant<uint, std::wstring_view>& character)
{
    const auto characterName = GetCharacterName(character);
    if (characterName.empty())
    {
        return { cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter)) };
    }
    return storage.count<MailItem>(where(::c(&MailItem::characterName) == characterName));
}

void MailManager::SendMailNotification(const std::variant<uint, std::wstring_view>& character)
{
    const auto client = Hk::Client::ExtractClientID(character);
    if (client == UINT_MAX)
    {
        return;
    }

    const auto mailCount = GetUnreadMailCount(client);
    if (mailCount.has_error())
    {
        Logger::i()->Log(LogLevel::Err, mailCount.error());
    }
    else if (mailCount.value() > 0)
    {
        client.Message(std::format(L"You have {} unread mail! Read it via /mailread.", mailCount.value()));
    }
}

void MailManager::CleanUpOldMail()
{
    auto mails = storage.get_all<MailItem>();

    for (auto m : mails)
    {
        const auto acc = Hk::Client::GetAccountByCharName(m.characterName).Raw();
        if (acc.has_error())
        {
            PurgeAllMail(m.characterName, false);
        }
    }
}
