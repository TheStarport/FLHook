#include "PCH.hpp"

#include "API/FLHook/Mail/MailManager.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/TaskScheduler.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

void MailManager::SendMailCallback(const std::shared_ptr<void>& taskData)
{
    const auto mailTargets = std::static_pointer_cast<std::vector<std::wstring>>(taskData);

    auto& clients = FLHook::Clients();
    for (const auto& character : *mailTargets)
    {
        for (const auto& client : clients)
        {
            if (client.characterName == character)
            {
                // Character is online, message them now!
                (void)client.id.Message(L"You have received a new mail item.");
                break;
            }
        }
    }
}

bool MailManager::DeferSendMail(const std::shared_ptr<void>& taskData, Mail mail)
{
    auto mailTargets = std::static_pointer_cast<std::vector<std::wstring>>(taskData);

    const auto config = FLHook::GetConfig();

    const auto dbClient = FLHook::GetDatabase().AcquireClient();
    auto mailCollection = dbClient->database(config.databaseConfig.dbName).collection(config.databaseConfig.mailCollection);

    auto docBuilder = bsoncxx::builder::basic::document{};
    docBuilder.append(kvp("message", mail.message));
    docBuilder.append(kvp("sentDate", bsoncxx::types::b_date{ static_cast<std::chrono::milliseconds>(TimeUtils::UnixTime<std::chrono::milliseconds>()) }));

    auto arrBuilder = bsoncxx::builder::basic::array{};
    for (const auto& recipient : mail.recipients)
    {
        arrBuilder.append(make_document(kvp("target", recipient.target)));
    }

    docBuilder.append(kvp("recipients", arrBuilder.view()));

    if (mail.origin.has_value())
    {
        docBuilder.append(kvp("origin", mail.origin.value()));
    }
    else
    {
        docBuilder.append(kvp("author", mail.author.value()));
    }

    try
    {
        const auto response = mailCollection.insert_one(docBuilder.view());
        assert(response.has_value());

        auto insertedId = response->inserted_id().get_oid();

        mongocxx::pipeline pipeline;
        pipeline.match(make_document(kvp("_id", insertedId)));

        pipeline.lookup(make_document(
            kvp("from", config.databaseConfig.accountsCollection), kvp("localField", "recipients.target"), kvp("foreignField", "_id"), kvp("as", "results")));

        pipeline.project(make_document(kvp("_id", 0), kvp("results", 1)));

        auto results = mailCollection.aggregate(pipeline);

        for (const auto result : results)
        {
            if (auto name = result.find("characterName"); name != result.end())
            {
                mailTargets->emplace_back(StringUtils::stows(name->get_string().value));
            }
        }
    }
    catch (const mongocxx::exception& ex)
    {
        Logger::Err(std::format(L"Error while trying to send mail. {}", StringUtils::stows(ex.what())));
    }

    return true;
}

bool MailManager::GetMailForCharacter(const std::shared_ptr<void>& taskData, bsoncxx::oid characterId, const int count, const int page, const bool newestFirst)
{
    const auto allMail = std::static_pointer_cast<std::vector<Mail>>(taskData);

    const auto client = FLHook::GetDatabase().AcquireClient();
    const auto& config = FLHook::GetConfig();

    auto mailCollection = client->database(config.databaseConfig.dbName).collection(config.databaseConfig.mailCollection);

    mongocxx::pipeline pipeline;

    // clang-format off
    pipeline
        .match(make_document(kvp("$expr", make_document(kvp("$in", make_array(characterId, "$recipients.target"))))))
        // Paginate
        .sort(make_document(kvp("sentDate", newestFirst ? -1 : 1)))
        .skip(count * page)
        .limit(count);
    // clang-format on

    for (const auto result : mailCollection.aggregate(pipeline))
    {
        Mail mail{};
        ParseMail(mail, result);

        if (!mail.message.empty() && !mail.recipients.empty())
        {
            allMail->emplace_back(mail);
        }
    }

    return true;
}

bool MailManager::GetMailForAccount(const std::shared_ptr<void>& taskData, AccountId accountId, const int count, const int page, const bool newestFirst)
{
    auto allMail = std::static_pointer_cast<std::vector<Mail>>(taskData);

    const auto client = FLHook::GetDatabase().AcquireClient();
    const auto& config = FLHook::GetConfig();

    auto mailCollection = client->database(config.databaseConfig.dbName).collection(config.databaseConfig.mailCollection);

    mongocxx::pipeline pipeline;

    // clang-format off
    pipeline
        .lookup(make_document(
            kvp("from", config.databaseConfig.accountsCollection),
            kvp("let", make_document(kvp("recipients", "$recipients"))),
            kvp("pipeline", make_array(
                make_document(kvp("$match", make_document(kvp("$expr",
                    // Check that the mail we are looking for has a matching account id and the same character id
                    make_document(kvp("$and", make_array(
                        make_document(kvp("$eq", make_array("$accountId", accountId.GetValue()))),
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

    for (auto result : mailCollection.aggregate(pipeline))
    {
        auto mailDoc = result.find("mail");
        if (mailDoc == result.end())
        {
            continue;
        }

        Mail mail;
        ParseMail(mail, mailDoc->get_document().view());

        if (!mail.message.empty() && !mail.recipients.empty())
        {
            allMail->emplace_back(mail);
        }
    }

    return true;
}

void MailManager::ParseMail(Mail& mail, const bsoncxx::document::view doc)
{
    for (const auto& m : doc)
    {
        switch (Hash(m.key().data()))
        {
            case Hash("_id"):
                {
                    mail._id = m.get_oid().value;
                    break;
                }
            case Hash("author"):
                {
                    mail.author = m.get_oid().value;
                    break;
                }
            case Hash("origin"):
                {
                    mail.origin = m.get_string().value;
                    break;
                }
            case Hash("sentDate"):
                {
                    mail.sentDate = m.get_date();
                    break;
                }
            case Hash("message"):
                {
                    mail.message = m.get_string().value;
                    break;
                }
            case Hash("recipients"):
                {
                    for (const auto& recipients = m.get_array(); auto doc : recipients.value)
                    {
                        MailRecipient recipient;
                        for (const auto& el : doc.get_document().view())
                        {
                            switch (Hash(el.key().data()))
                            {
                                case Hash("target"):
                                    {
                                        recipient.target = el.get_oid().value;
                                        break;
                                    }
                                case Hash("readDate"):
                                    {
                                        recipient.readDate = el.get_date();
                                        break;
                                    }
                                default: break;
                            }
                        }

                        mail.recipients.emplace_back(recipient);
                    }

                    break;
                }
            default: break;
        }
    }
}

void MailManager::GetAccountMail(const AccountId& id, std::function<void(const std::shared_ptr<std::vector<Mail>>&)> callback, int count, int page, bool newestFirst)
{
    auto data = std::make_shared<std::vector<Mail>>();

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

    TaskScheduler::ScheduleWithCallback<std::vector<Mail>>(std::bind(GetMailForAccount, std::placeholders::_1, id, count, page, newestFirst),
        [callback](const std::shared_ptr<void>& taskData) { callback(std::static_pointer_cast<std::vector<Mail>>(taskData)); });
}

void MailManager::GetCharacterMail(bsoncxx::oid characterId, std::function<void(const std::shared_ptr<std::vector<Mail>>&)> callback, int count, int page, bool newestFirst)
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

    TaskScheduler::ScheduleWithCallback<std::vector<Mail>>(std::bind(GetMailForCharacter, std::placeholders::_1, characterId, count, page, newestFirst),
                                                           [callback](const std::shared_ptr<void>& taskData) { callback(std::static_pointer_cast<std::vector<Mail>>(taskData)); });
}

void MailManager::MarkMailAsRead(const Mail& mail, bsoncxx::oid character)
{
    auto mailId = mail._id;
    TaskScheduler::Schedule(
        [mailId, character]
        {
            const auto config = FLHook::GetConfig();

            const auto dbClient = FLHook::GetDatabase().AcquireClient();
            auto mailCollection = dbClient->database(config.databaseConfig.dbName).collection(config.databaseConfig.mailCollection);

            mailCollection.update_one(
                make_document(kvp("$and",
                                  make_array(make_document(kvp("_id", make_document(kvp("$eq", mailId)))),
                                             make_document(kvp("recipients.target", make_document(kvp("$eq", character))))))),
                make_document(kvp(
                    "$set",
                    make_document(kvp("recipients.$.readDate",
                                      bsoncxx::types::b_date{ static_cast<std::chrono::milliseconds>(TimeUtils::UnixTime<std::chrono::milliseconds>()) })))));
        });
}

Action<void, Error> MailManager::SendMail(const Mail& mail)
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

    TaskScheduler::ScheduleWithCallback<std::vector<std::wstring>>(std::bind(DeferSendMail, std::placeholders::_1, mail), SendMailCallback);

    return { {} };
}
