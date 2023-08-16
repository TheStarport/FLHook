#pragma once

#include "API/Utils/FileUtils.hpp"

#include <sqlite_orm/sqlite_orm.h>

class DLL MailManager : public Singleton<MailManager>
{
        [[nodiscard]]
        std::wstring_view GetCharacterName(const std::variant<uint, std::wstring_view>& character) const;

    public:
        struct MailItem
        {
                int64 id{};
                bool unread{};
                std::wstring subject;
                std::wstring author;
                std::wstring body;
                int64 timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                std::wstring characterName;
        };

    private:
        sqlite_orm::internal::storage_t<sqlite_orm::internal::table_t<
            MailItem, false,
            sqlite_orm::internal::column_t<long long MailItem::*, sqlite_orm::internal::empty_setter,
                                           sqlite_orm::internal::primary_key_with_autoincrement<sqlite_orm::internal::primary_key_t<>>>,
            sqlite_orm::internal::column_t<bool MailItem::*, sqlite_orm::internal::empty_setter, sqlite_orm::internal::default_t<bool>>,
            sqlite_orm::internal::column_t<std::wstring MailItem::*, sqlite_orm::internal::empty_setter>,
            sqlite_orm::internal::column_t<std::wstring MailItem::*, sqlite_orm::internal::empty_setter>,
            sqlite_orm::internal::column_t<std::wstring MailItem::*, sqlite_orm::internal::empty_setter>,
            sqlite_orm::internal::column_t<long long MailItem::*, sqlite_orm::internal::empty_setter>,
            sqlite_orm::internal::column_t<std::wstring MailItem::*, sqlite_orm::internal::empty_setter>>>
            storage = sqlite_orm::make_storage(
                StringUtils::wstos(FileUtils::SaveDataPath()) + "mail.db",
                sqlite_orm::make_table("mailItems", sqlite_orm::make_column("id", &MailItem::id, sqlite_orm::primary_key().autoincrement()),
                                       sqlite_orm::make_column("unread", &MailItem::unread, sqlite_orm::default_value(true)),
                                       sqlite_orm::make_column("subject", &MailItem::subject), sqlite_orm::make_column("author", &MailItem::author),
                                       sqlite_orm::make_column("body", &MailItem::body), sqlite_orm::make_column("timestamp", &MailItem::timestamp),
                                       sqlite_orm::make_column("characterName", &MailItem::characterName)));

        enum class ErrorTypes
        {
            InvalidCharacter,
            SubjectTooLong,
            AuthorTooLong,
            BodyTooLong,
            MailIdNotFound
        };

        [[nodiscard]]
        static constexpr std::wstring GetErrorCode(ErrorTypes err)
        {
            std::vector<std::wstring> errors = { L"Invalid character name or client id",
                                                 L"Subject cannot be longer than 32 characters.",
                                                 L"Author cannot be longer than 36 characters.",
                                                 L"Body cannot be longer than 255 characters.",
                                                 L"Mail id was not found." };
            return errors[static_cast<int>(err)];
        }

    public:
        MailManager();

        /// <summary>
        /// Sends a mail to the target character, specified by ClientId or string.
        /// </summary>
        /// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character
        /// name.</param> <param name="item">The preconstructed mail object.</param> <returns>Nothing in the event of success, otherwise an std::wstring of an
        /// err</returns>
        cpp::result<void, std::wstring> SendNewMail(const std::variant<uint, std::wstring_view>& character, const MailItem& item);

        /// <summary>
        /// Gets a list of mail from the target character, ordered by most recent.
        /// </summary>
        /// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character
        /// name.</param> <param name="ignoreRead">If true, mail that has already been read is skipped.</param> <param name="page">Mail will be returned in
        /// batches of 15. The page will indicate which batch should be returned.</param> <returns>An std::vector of MailItem in s returned in the event of
        /// success, otherwise an std::wstring of an err.</returns>
        cpp::result<std::vector<MailItem>, std::wstring> GetMail(const std::variant<uint, std::wstring_view>& character, const bool& ignoreRead,
                                                                 const int& page);

        /// <summary>
        /// Get's the associated mail item by accessing it via ID and character name.
        /// </summary>
        /// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character
        /// name.</param> <param name="index">The mail item number.</param> <returns>A mail item in the event of success, otherwise an std::wstring of
        /// err</returns>
        cpp::result<MailItem, std::wstring> GetMailById(const std::variant<uint, std::wstring_view>& character, const int64& index);

        /// <summary>
        /// Delete mail specified by the mail number.
        /// </summary>
        /// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character
        /// name.</param> <param name="index">The mail item number.</param> <returns>Nothing in the event of success, otherwise an std::wstring of an
        /// err</returns>
        cpp::result<void, std::wstring> DeleteMail(const std::variant<uint, std::wstring_view>& character, const int64& index);

        /// <summary>
        /// Delete all mail on the character specified
        /// </summary>
        /// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character
        /// name.</param> <param name="readMailOnly">If true, only mail that has been read will be purged.</param> <returns>An int64 listing the amount of mail
        /// deleted in the event of success, otherwise an std::wstring of an err</returns>
        cpp::result<int64, std::wstring> PurgeAllMail(const std::variant<uint, std::wstring_view>& character, const bool& readMailOnly);

        /// <summary>
        /// Changes all references to a character name within the database in the event of it being changed.
        /// </summary>
        /// <param name="oldCharacterName">The old name of the character.</param>
        /// <param name="newCharacterName">The new name of the character, this command will error if the name doesn't currently exist on a character.</param>
        // <returns>An int64 listing the number of mail items updated in the event of success, otherwise an std::wstring of an err</returns>
        cpp::result<int64, std::wstring> UpdateCharacterName(const std::wstring& oldCharacterName, const std::wstring& newCharacterName);

        /// <summary>
        /// Returns a number indicating the amount of mail items this character has that have not been read.
        /// </summary>
        /// <param name="character">The client id or character name.</param>
        // <returns>An int64 listing the number of mail items that are unread in the event of success, otherwise an std::wstring of an err</returns>
        cpp::result<int64, std::wstring> GetUnreadMailCount(const std::variant<uint, std::wstring_view>& character);

        /// <summary>
        /// Informs the user if they have unread mail.
        /// </summary>
        /// <param name="character">The client id or character name.</param>
        void SendMailNotification(const std::variant<uint, std::wstring_view>& character);

        /// <summary>
        /// Remove all mail that no longer have valid character references
        /// </summary>
        void CleanUpOldMail();
};
