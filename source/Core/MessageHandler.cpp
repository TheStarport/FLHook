#include "PCH.hpp"

#include "Core/MessageHandler.hpp"

#include "API/FLHook/MessageInterface.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"
#include "Defs/ServerStats.hpp"

#include <Psapi.h>

void MessageHandler::PublishServerStats()
{
    ServerStats stats;

    stats.npcsEnabled = InternalApi::NpcsEnabled();
    stats.serverLoad = FLHook::GetServerLoadInMs();

    PROCESS_MEMORY_COUNTERS memCounter;
    GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof memCounter);
    stats.memoryUsage = memCounter.WorkingSetSize;

    for (auto& client : FLHook::Clients())
    {
        if (client.characterName.empty())
        {
            continue;
        }

        auto system = client.id.GetSystemId().Unwrap();

        ServerStats::Player player;
        player.clientId = client.id.GetValue();
        player.playerName = StringUtils::wstos(client.characterName);
        player.systemName = system.GetValue() ? StringUtils::wstos(system.GetName().Unwrap()) : "";
        player.systemNick = system.GetValue() ? StringUtils::wstos(system.GetNickName().Unwrap()) : "";
        player.ipAddress = StringUtils::wstos(client.id.GetPlayerIp().Unwrap());

        stats.players.emplace_back(player);
    }

    const auto bson = rfl::bson::write(stats);
    FLHook::GetMessageInterface()->Publish(
        std::string_view(bson.data(), bson.size()), std::wstring(MessageInterface::QueueToStr(MessageInterface::Queue::ServerStats)), L"");
}

bool MessageHandler::ProcessExternalCommand(const AMQP::Message& message, std::shared_ptr<BsonWrapper>& response)
{
    // TODO: ensure this runs on the main thread and therefore is safe to manipulate players
    const std::string_view body = { message.body(), message.body() + message.bodySize() };

    try
    {
        const BsonWrapper bsonWrapper(body);
        const auto bson = bsonWrapper.GetValue();
        if (!bson.has_value())
        {
            return false;
        }

        if (const auto [successful, responseDoc] = ExternalCommandProcessor::i()->ProcessCommand(bson.value()); successful || responseDoc)
        {
            response = responseDoc;
            return true;
        }

        return false;
    }
    catch (GameException& ex)
    {
        Logger::Err(std::format(L"Exception while processing external command: {}", ex.Msg()));
        return true;
    }
    catch (std::exception& ex)
    {
        return true;
    }
}

MessageHandler::MessageHandler()
{
    const auto config = FLHook::GetConfig();
    auto msg = FLHook::GetMessageInterface();

    // Declared needed queues
    msg->DeclareExchange(std::wstring(MessageInterface::QueueToStr(MessageInterface::Queue::ServerStats)), AMQP::fanout, AMQP::durable);
    msg->DeclareQueue(std::wstring(MessageInterface::QueueToStr(MessageInterface::Queue::ExternalCommands)), AMQP::durable);

    // Setup subscriptions
    msg->Subscribe(std::wstring(MessageInterface::QueueToStr(MessageInterface::Queue::ExternalCommands)), ProcessExternalCommand);

    // Setup exchanges and timers
    Timer::Add(PublishServerStats, config->messageQueue.timeBetweenServerUpdates);
}

MessageHandler::~MessageHandler() {}
