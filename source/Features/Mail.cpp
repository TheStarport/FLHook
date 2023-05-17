#include "PCH.hpp"
#include "Features/Mail.hpp"
#include "Global.hpp"
#include "Helpers/Client.hpp"


std::string MailManager::GetCharacterName(const std::variant<uint, std::wstring>& character) const
{
	// If character is uint
	if (!character.index())
	{
		const auto name = Hk::Client::GetCharacterNameByID(std::get<uint>(character));
		if (name.has_error())
			return "";

		return wstos(name.value());
	}

	// Validate that the name is correct
	if (const auto acc = Hk::Client::GetAccountByCharName(std::get<std::wstring>(character)); acc.has_error())
		return "";

	return wstos(std::get<std::wstring>(character));
}

MailManager::MailManager()
{
	if (db.tableExists("items"))
	{
		return;
	}

	db.exec("CREATE TABLE items ("
		"id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,"
		"unread INTEGER(1, 1) NOT NULL CHECK(unread IN(0, 1)),"
		"subject TEXT(36, 36) NOT NULL,"
		"author TEXT(36, 36) NOT NULL,"
		"body TEXT(255, 255) NOT NULL,"
		"timestamp INTEGER NOT NULL,"
		"characterName TEXT(36, 36) NOT NULL);");

	db.exec("CREATE INDEX IDX_characterName ON items (characterName DESC);");
}

cpp::result<void, std::string> MailManager::SendNewMail(const std::variant<uint, std::wstring>& character, const MailItem& item) const
{
	const auto characterName = GetCharacterName(character);
	if (characterName.empty())
	{
		return cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter));
	}

	if (item.subject.length() > 32)
	{
		return cpp::fail(GetErrorCode(ErrorTypes::SubjectTooLong));
	}

	if (item.author.length() > 36)
	{
		return cpp::fail(GetErrorCode(ErrorTypes::AuthorTooLong));
	}

	if (item.body.length() > 255)
	{
		return cpp::fail(GetErrorCode(ErrorTypes::BodyTooLong));
	}

	SQLite::Statement newMailQuery(db, "INSERT INTO items (unread, subject, author, body, characterName, timestamp) VALUES(?, ?, ?, ?, ?, ?)");
	newMailQuery.bind(1, item.unread);
	newMailQuery.bind(2, item.subject);
	newMailQuery.bind(3, item.author);
	newMailQuery.bind(4, item.body);
	newMailQuery.bind(5, characterName);
	newMailQuery.bind(6, item.timestamp);

	try
	{
		newMailQuery.exec();
	}
	catch (SQLite::Exception ex)
	{
		return cpp::fail(ex.getErrorStr());
	}

	return {};
}

using namespace std::string_literals;

cpp::result<std::vector<MailManager::MailItem>, std::string> MailManager::GetMail(
	const std::variant<uint, std::wstring>& character, const bool& ignoreRead, const int& page)
{
	const auto characterName = GetCharacterName(character);
	if (characterName.empty())
	{
		return cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter));
	}

	if (ignoreRead)
	{
		SQLite::Statement getUnreadMail(
			db,
			"SELECT id, subject, author, body, timestamp FROM items WHERE unread = TRUE AND characterName = ? ORDER BY timestamp DESC LIMIT 15 OFFSET ?;");
		getUnreadMail.bind(1, characterName);
		getUnreadMail.bind(2, (page - 1) * 15);

		std::vector<MailItem> mail;
		while (getUnreadMail.executeStep())
		{
			mail.emplace_back(getUnreadMail.getColumn(0).getInt64(),
				true,
				getUnreadMail.getColumn(1).getString(),
				getUnreadMail.getColumn(2).getString(),
				getUnreadMail.getColumn(3).getString(),
				getUnreadMail.getColumn(4).getInt64());
		}

		return mail;
	}

	SQLite::Statement getMail(db,
		"SELECT id, unread, subject, author, body, timestamp FROM items WHERE characterName = ? ORDER BY timestamp DESC LIMIT 15 OFFSET ?;");
	getMail.bind(1, characterName);
	getMail.bind(2, (page - 1) * 15);

	std::vector<MailItem> mail;
	while (getMail.executeStep())
	{
		mail.emplace_back(getMail.getColumn(0).getInt64(),
			static_cast<bool>(getMail.getColumn(1).getInt()),
			getMail.getColumn(2).getString(),
			getMail.getColumn(3).getString(),
			getMail.getColumn(4).getString(),
			getMail.getColumn(5).getInt64());
	}

	return mail;
}

cpp::result<MailManager::MailItem, std::string> MailManager::GetMailById(const std::variant<uint, std::wstring>& character, const int64& index)
{
	const auto characterName = GetCharacterName(character);
	if (characterName.empty())
	{
		return cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter));
	}

	SQLite::Statement getMail(db, "SELECT unread, subject, author, body, timestamp FROM items WHERE characterName = ? AND id = ?;");
	getMail.bind(1, characterName);
	getMail.bind(2, index);

	MailItem mail;

	try
	{
		if (!getMail.executeStep())
		{
			return cpp::fail(GetErrorCode(ErrorTypes::MailIdNotFound));
		}

		mail = MailItem(index,
			static_cast<bool>(getMail.getColumn(0).getInt()),
			getMail.getColumn(1).getString(),
			getMail.getColumn(2).getString(),
			getMail.getColumn(3).getString(),
			getMail.getColumn(4).getInt64());

		SQLite::Statement markRead(db, "UPDATE items SET unread = FALSE WHERE id = ?;");
		markRead.bind(1, index);
		markRead.exec();
		return mail;
	}
	catch (SQLite::Exception ex)
	{
		return cpp::fail(ex.getErrorStr());
	}
}

cpp::result<void, std::string> MailManager::DeleteMail(const std::variant<uint, std::wstring>& character, const int64& index) const
{
	const auto characterName = GetCharacterName(character);
	if (characterName.empty())
	{
		return cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter));
	}

	SQLite::Statement deleteMail(db, "DELETE FROM items WHERE id = ? AND characterName = ?;");
	deleteMail.bind(1, index);
	deleteMail.bind(2, characterName);

	try
	{
		if (!deleteMail.exec())
		{
			return cpp::fail(GetErrorCode(ErrorTypes::MailIdNotFound));
		}
	}
	catch (SQLite::Exception ex)
	{
		return cpp::fail(ex.getErrorStr());
	}

	return {};
}

cpp::result<int64, std::string> MailManager::PurgeAllMail(const std::variant<uint, std::wstring>& character, const bool& readMailOnly)
{
	const auto characterName = GetCharacterName(character);
	if (characterName.empty())
	{
		return cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter));
	}

	SQLite::Statement deleteMail = readMailOnly
		? SQLite::Statement(db, "DELETE FROM items WHERE unread = FALSE AND characterName = ?;")
		: SQLite::Statement(db, "DELETE FROM items WHERE characterName = ?;");
	deleteMail.bind(1, characterName);

	try
	{
		int count = deleteMail.exec();
		if (!count)
		{
			return cpp::fail("No mail to delete.");
		}

		return count;
	}
	catch (SQLite::Exception ex)
	{
		return cpp::fail(ex.getErrorStr());
	}
}

cpp::result<int64, std::string> MailManager::UpdateCharacterName(const std::string& oldCharacterName, const std::string& newCharacterName)
{
	if (const auto acc = Hk::Client::GetAccountByCharName(stows(newCharacterName)); acc.has_error())
	{
		return cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter));
	}

	SQLite::Statement updateName(db, "UPDATE items SET characterName = ? WHERE characterName = ?;");
	updateName.bind(1, newCharacterName);
	updateName.bind(2, oldCharacterName);

	try
	{
		int count = updateName.exec();
		if (!count)
		{
			return cpp::fail(GetErrorCode(ErrorTypes::MailIdNotFound));
		}

		return count;
	}
	catch (SQLite::Exception ex)
	{
		return cpp::fail(ex.getErrorStr());
	}
}

cpp::result<int64, std::string> MailManager::GetUnreadMail(const std::variant<uint, std::wstring>& character)
{
	const auto characterName = GetCharacterName(character);
	if (characterName.empty())
	{
		return cpp::fail(GetErrorCode(ErrorTypes::InvalidCharacter));
	}

	SQLite::Statement count(db, "SELECT COUNT(*) FROM items WHERE characterName = ?;");
	count.bind(1, characterName);

	if (count.executeStep())
	{
		return count.getColumn(0).getInt64();
	}

	return 0;
}

void MailManager::SendMailNotification(const std::variant<uint, std::wstring>& character)
{
	const auto client = Hk::Client::ExtractClientID(character);
	if (client == UINT_MAX)
	{
		return;
	}

	const auto mailCount = GetUnreadMail(client);
	if (mailCount.has_error())
	{
		Logger::i()->Log(LogLevel::Err, mailCount.error());
	}
	else if (mailCount.value() > 0)
	{
		PrintUserCmdText(client, std::format(L"You have {} unread mail! Read it via /mailread.", mailCount.value()));
	}
}

void MailManager::CleanUpOldMail()
{
	try
	{
		SQLite::Statement getAllUniqueCharacterNames(db, "SELECT characterName FROM items GROUP BY characterName;");
		while (getAllUniqueCharacterNames.executeStep())
		{
			const auto name = stows(getAllUniqueCharacterNames.getColumn(0));
			const auto acc = Hk::Client::GetAccountByCharName(name);
			if (acc.has_error())
			{
				PurgeAllMail(name, false);
			}
		}
	}
	catch (SQLite::Exception ex)
	{
		Logger::i()->Log(LogLevel::Err, std::format("Unable to perform mail cleanup. Err: {}", ex.getErrorStr()));
	}
}

