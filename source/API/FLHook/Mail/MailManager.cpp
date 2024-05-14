#include "PCH.hpp"

#include "API/FLHook/Mail/MailManager.hpp"

#include "API/FLHook/Database.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "API/FLHook/ClientList.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

void MailManager::SendMailCallback(const int id)
{
    std::scoped_lock lock(mailCallbackMutex);

    const auto inflight = inflightMail.find(id);
    if (inflight == inflightMail.end())
    {
        return;
    }

    auto& clients = FLHook::Clients();
    for (const auto& character : inflight->second)
    {
        for (const auto& client : clients)
        {
            if (client.characterName == character)
            {
                // Character is online, message them now!
                (void)client.id.Message(L"You have recieved a new mail item.");
                break;
            }
        }
    }

    inflightMail.erase(inflight);
}

void MailManager::GetAccountMail(const AccountId& id, std::function<void(std::vector<Mail>)> callback, int count, int page, bool newestFirst) {}
void MailManager::GetCharacterMail(bsoncxx::oid characterId, std::function<void(std::optional<Mail>)> callback, int count, int page, bool newestFirst) {}

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

    int id = inflightMailId++;
    TaskScheduler::ScheduleWithCallback(
        [mail, id]
        {
            const auto config = FLHook::GetConfig();

            const auto dbClient = FLHook::GetDatabase().AcquireClient();
            auto mailCollection = dbClient->database(config.databaseConfig.dbName).collection(config.databaseConfig.mailCollection);

            auto docBuilder = bsoncxx::builder::basic::document{};
            docBuilder.append(kvp("message", mail.message));
            docBuilder.append(
                kvp("sentDate", bsoncxx::types::b_date{ static_cast<std::chrono::milliseconds>(TimeUtils::UnixTime<std::chrono::milliseconds>()) }));

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

                pipeline.lookup(make_document(kvp("from", config.databaseConfig.accountsCollection),
                                              kvp("localField", "recipients.target"),
                                              kvp("foreignField", "_id"),
                                              kvp("as", "results")));

                pipeline.project(make_document(kvp("_id", 0), kvp("results", 1)));

                auto results = mailCollection.aggregate(pipeline);

                // Lock the mutex, ensure we can insert to our inflight mail
                std::scoped_lock lock(mailCallbackMutex);

                auto& inflight = inflightMail[id] = {};
                for (const auto result : results)
                {
                    if (auto name = result.find("characterName"); name != result.end())
                    {
                        inflight.emplace_back(StringUtils::stows(name->get_string().value));
                    }
                }
            }
            catch (const mongocxx::exception& ex)
            {
                Logger::Err(std::format(L"Error while trying to send mail. {}", StringUtils::stows(ex.what())));
            }
        },
        std::bind(SendMailCallback, id));

    return { {} };
}
