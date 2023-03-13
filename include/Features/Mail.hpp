#pragma once

#include <FLHook.hpp>

class DLL MailManager : public Singleton<MailManager>
{
	std::string GetCharacterName(const std::variant<uint, std::wstring>& character) const;

public:
	struct MailItem
	{
		int64 id;
		bool unread = true;
		std::string subject;
		std::string author;
		std::string body;
		int64 timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		std::string characterName;
	};

private:
	SQLite::Database db = SqlHelpers::Create("mail.sqlite");

	enum class ErrorTypes
	{
		InvalidCharacter,
		SubjectTooLong,
		AuthorTooLong,
		BodyTooLong,
		MailIdNotFound
	};

	constexpr std::string GetErrorCode(ErrorTypes err) const
	{
		std::vector<std::string> errors = {"Invalid character name or client id",
		                                   "Subject cannot be longer than 32 characters.",
		                                   "Author cannot be longer than 36 characters.",
		                                   "Body cannot be longer than 255 characters.",
		                                   "Mail id was not found."
		};
		return errors[static_cast<int>(err)];
	}

public:
	MailManager();

	/// <summary>
	/// Sends a mail to the target character, specified by ClientId or string.
	/// </summary>
	/// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character name.</param>
	/// <param name="item">The preconstructed mail object.</param>
	/// <returns>Nothing in the event of success, otherwise an std::string of an err</returns>
	cpp::result<void, std::string> SendNewMail(const std::variant<uint, std::wstring>& character, const MailItem& item) const;

	/// <summary>
	/// Gets a list of mail from the target character, ordered by most recent.
	/// </summary>
	/// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character name.</param>
	/// <param name="ignoreRead">If true, mail that has already been read is skipped.</param>
	/// <param name="page">Mail will be returned in batches of 15. The page will indicate which batch should be returned.</param>
	/// <returns>An std::vector of MailItem in s returned in the event of success, otherwise an std::string of an err.</returns>
	cpp::result<std::vector<MailItem>, std::string> GetMail(const std::variant<uint, std::wstring>& character, const bool& ignoreRead, const int& page);

	/// <summary>
	/// Get's the associated mail item by accessing it via ID and character name.
	/// </summary>
	/// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character name.</param>
	/// <param name="index">The mail item number.</param>
	/// <returns>A mail item in the event of success, otherwise an std::string of err</returns>
	cpp::result<MailItem, std::string> GetMailById(const std::variant<uint, std::wstring>& character, const int64& index);

	/// <summary>
	/// Delete mail specified by the mail number.
	/// </summary>
	/// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character name.</param>
	/// <param name="index">The mail item number.</param>
	/// <returns>Nothing in the event of success, otherwise an std::string of an err</returns>
	cpp::result<void, std::string> DeleteMail(const std::variant<uint, std::wstring>& character, const int64& index) const;

	/// <summary>
	/// Delete all mail on the character specified
	/// </summary>
	/// <param name="character">If a uint specified, it calculates to the id of the online user. Otherwise a string is used for their character name.</param>
	/// <param name="readMailOnly">If true, only mail that has been read will be purged.</param>
	/// <returns>An int64 listing the amount of mail deleted in the event of success, otherwise an std::string of an err</returns>
	cpp::result<int64, std::string> PurgeAllMail(const std::variant<uint, std::wstring>& character, const bool& readMailOnly);

	/// <summary>
	/// Changes all references to a character name within the database in the event of it being changed.
	/// </summary>
	/// <param name="oldCharacterName">The old name of the character.</param>
	/// <param name="newCharacterName">The new name of the character, this command will error if the name doesn't currently exist on a character.</param>
	// <returns>An int64 listing the number of mail items updated in the event of success, otherwise an std::string of an err</returns>
	cpp::result<int64, std::string> UpdateCharacterName(const std::string& oldCharacterName, const std::string& newCharacterName);

	/// <summary>
	/// Returns a number indicating the amount of mail items this character has that have not been read.
	/// </summary>
	/// <param name="character">The client id or character name.</param>
	// <returns>An int64 listing the number of mail items that are unread in the event of success, otherwise an std::string of an err</returns>
	cpp::result<int64, std::string> GetUnreadMail(const std::variant<uint, std::wstring>& character);

	/// <summary>
	/// Informs the user if they have unread mail.
	/// </summary>
	/// <param name="character">The client id or character name.</param>
	void SendMailNotification(const std::variant<uint, std::wstring>& character);

	/// <summary>
	/// Remove all mail that no longer have valid character references
	/// </summary>
	void CleanUpOldMail();
};
