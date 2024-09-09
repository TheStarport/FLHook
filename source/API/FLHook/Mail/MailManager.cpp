#include "PCH.hpp"

#include "API/FLHook/Mail/MailManager.hpp"

#include "API/Utils/Reflection.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/TaskScheduler.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

Task MailManager::InformOnlineUsersOfNewMail(std::vector<rfl::Variant<bsoncxx::oid, std::string>> accountIdOrCharacterNames) // NOLINT(*-unnecessary-value-param)
{
    co_yield TaskStatus::FLHookAwait;

    auto& clients = FLHook::Clients();
    for (const auto& variant : accountIdOrCharacterNames)
    {
        for (const auto& client : clients)
        {
            if ((!variant.index() && client.characterData->_id.value() == rfl::get<0>(variant)) ||
                (variant.index() && client.account->_id == rfl::get<1>(variant)))
            {
                // Character is online, message them now!
                (void)client.id.Message(std::format(L"You have received {} mail.", variant.index()
                    ? L"character"
                    : L"account"));
                break;
            }
        }
    }

    co_return TaskStatus::Finished;
}

Action<std::vector<Mail>, Error> MailManager::GetAccountMail(std::string accountId, int count, int page, bool newestFirst) // NOLINT(*-unnecessary-value-param)
{
    if (page < 1)
    {
        page = 0;
    }
    else
    {
        page--;
    }

    if (count < 1)
    {
        count = 1;
    }
    else if (count > 50)
    {
        count = 50;
    }

    const auto client = FLHook::GetDatabase().AcquireClient();
    const auto& config = FLHook::GetConfig();

    auto mailCollection = client->database(config.database.dbName).collection(config.database.mailCollection);

    mongocxx::pipeline pipeline;

    // clang-format off
    pipeline
        .lookup(make_document(
            kvp("from", config.database.accountsCollection),
            kvp("let", make_document(kvp("recipients", "$recipients"))),
            kvp("pipeline", make_array(
                make_document(kvp("$match", make_document(kvp("$expr",
                    // Check that the mail we are looking for has a matching account id and the same character id
                    make_document(kvp("$and", make_array(
                        make_document(kvp("$eq", make_array("$accountId", accountId))),
                        make_document(kvp("$in", make_array("$_id", "$$recipients.target")))
                    )))
                ))))
            )),
            kvp("as", "mail")
        ))
        // Remove results that didn't match
        .match(make_document(kvp("mail.0", make_document(kvp("$exists", true)))))
        // Paginate
        .sort(make_document(kvp("sentDate", newestFirst ? -1 : 1)))
        .skip(count * page)
        .limit(count);
    // clang-format on

    try
    {
        std::vector<Mail> allMail;
        for (auto result : mailCollection.aggregate(pipeline))
        {
            if (auto mailDoc = result.find("mail"); mailDoc == result.end())
            {
                continue;
            }

            auto mailResult = rfl::bson::read<Mail>(result.data(), result.length());
            if (mailResult.error().has_value())
            {
                Logger::Err(std::format(L"Error while trying to read mail for character: {}", StringUtils::stows(accountId)));
                continue;
            }

            const Mail& mail = mailResult.value();
            if (!mail.message.empty() && !mail.recipients.empty())
            {
                allMail.emplace_back(mail);
            }
        }

        if (allMail.empty())
        {
            return { cpp::fail(Error::UnknownError) };
        }

        return { allMail };
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Err(std::format(L"Unable to aggregate account mail query: {}", StringUtils::stows(ex.what())));
        return { cpp::fail(Error::UnknownError) };
    }
}

Action<std::vector<Mail>, Error> MailManager::GetCharacterMail(bsoncxx::oid characterId, int count, int page, bool newestFirst)
{
    if (page < 1)
    {
        page = 0;
    }
    else
    {
        page--;
    }

    if (count < 1)
    {
        count = 1;
    }
    else if (count > 50)
    {
        count = 50;
    }

    const auto client = FLHook::GetDatabase().AcquireClient();
    const auto& config = FLHook::GetConfig();

    auto mailCollection = client->database(config.database.dbName).collection(config.database.mailCollection);

    mongocxx::pipeline pipeline;

    // clang-format off
    pipeline
        .match(make_document(kvp("$expr", make_document(kvp("$in", make_array(characterId, "$recipients.target"))))))
        // Paginate
        .sort(make_document(kvp("sentDate", newestFirst ? -1 : 1)))
        .skip(count * page)
        .limit(count);
    // clang-format on

    try
    {
        std::vector<Mail> allMail;
        for (const auto result : mailCollection.aggregate(pipeline))
        {
            auto mail = rfl::bson::read<Mail>(result.data(), result.length());
            if (mail.error().has_value())
            {
                Logger::Err(std::format(L"Error while trying to read mail for character: {}", StringUtils::stows(characterId.to_string())));
                continue;
            }

            if (!mail.value().message.empty() && !mail.value().recipients.empty())
            {
                allMail.emplace_back(mail.value());
            }
        }

        if (allMail.empty())
        {
            return { cpp::fail(Error::UnknownError) };
        }

        return { allMail };
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Err(std::format(L"Unable to aggregate account mail query: {}", StringUtils::stows(ex.what())));
        return { cpp::fail(Error::UnknownError) };
    }
}

Action<void, Error> MailManager::DeleteMail(const Mail& mail)
{
    const auto& config = FLHook::GetConfig();

    const auto dbClient = FLHook::GetDatabase().AcquireClient();
    auto mailCollection = dbClient->database(config.database.dbName).collection(config.database.mailCollection);

    try
    {
        if (const auto result = mailCollection.delete_one(make_document(kvp("_id", mail._id))); result.has_value() && result.value().deleted_count())
        {
            return { {} };
        }
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Err(std::format(L"Unable to delete mail: {}", StringUtils::stows(ex.what())));
    }

    return { cpp::fail(Error::UnknownError) };
}

Action<void, Error> MailManager::MarkMailAsRead(const Mail& mail, rfl::Variant<std::string, bsoncxx::oid> characterOrAccount)
{
    const auto& config = FLHook::GetConfig();

    const auto dbClient = FLHook::GetDatabase().AcquireClient();
    auto mailCollection = dbClient->database(config.database.dbName).collection(config.database.mailCollection);

    auto targetEq = characterOrAccount.index()
        ? make_document(kvp("$eq", rfl::get<bsoncxx::oid>(characterOrAccount)))
        : make_document(kvp("$eq", rfl::get<std::string>(characterOrAccount)));

    try
    {
        // clang-format off
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto result = mailCollection.update_one(
            make_document(kvp("$and", make_array(
                      make_document(kvp("_id", make_document(kvp("$eq", mail._id)))),
                      make_document(kvp("recipients.target", targetEq))))),
            make_document(kvp("$set", make_document(kvp("recipients.$.readDate",
                bsoncxx::types::b_date{ static_cast<std::chrono::milliseconds>(TimeUtils::UnixTime<std::chrono::milliseconds>()) })))));
        // clang-format on

        if (result.has_value())
        {
            if (result->modified_count())
            {
                return { {} };
            }

            // TODO: Already read mail? Return a different error code or nothing I guess?
        }
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Err(std::format(L"Unable to mark mail as read: {}", StringUtils::stows(ex.what())));
    }

    return { cpp::fail(Error::UnknownError) };
}

Action<void, Error> MailManager::SendMail(Mail& mail)
{
    if (mail._id.bytes() != nullptr)
    {
        Logger::Warn(L"Sending mail that already has an ID is invalid!");
        return { cpp::fail(Error::UnknownError) };
    }

    if (!mail.author.has_value() && !mail.origin.has_value())
    {
        Logger::Warn(L"Cannot send mail that has no origin and no author!");
        return { cpp::fail(Error::UnknownError) };
    }

    if (mail.recipients.empty())
    {
        Logger::Warn(L"Cannot send mail with no designated recipients!");
        return { cpp::fail(Error::UnknownError) };
    }

    const auto& config = FLHook::GetConfig();

    const auto dbClient = FLHook::GetDatabase().AcquireClient();
    auto mailCollection = dbClient->database(config.database.dbName).collection(config.database.mailCollection);
    mail.sentDate = TimeUtils::MakeUtcTm(std::chrono::system_clock::now());

    const auto mailRaw = rfl::bson::write(mail);
    const auto mailDoc = bsoncxx::document::view(reinterpret_cast<const uint8_t*>(mailRaw.data()), mailRaw.size());

    std::vector<rfl::Variant<bsoncxx::oid, std::string>> mailTargets;

    try
    {
        const auto response = mailCollection.insert_one(mailDoc);
        assert(response.has_value());

        for (auto& [target, readDate] : mail.recipients)
        {
            mailTargets.emplace_back(target);
        }
    }
    catch (const mongocxx::exception& ex)
    {
        Logger::Err(std::format(L"Error while trying to send mail. {}", StringUtils::stows(ex.what())));
    }

    FLHook::GetTaskScheduler().AddTask(std::make_shared<Task>(InformOnlineUsersOfNewMail(mailTargets)));

    return { {} };
}
