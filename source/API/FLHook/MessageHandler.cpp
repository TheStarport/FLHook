#include "PCH.hpp"

#include "Core/MessageHandler.hpp"
#include "Defs/FLHookConfig.hpp"
#include <API/Utils/Logger.hpp>

MessageHandler::MessageHandler()
{
    if (!FLHook::GetConfig().messageQueue.enableQueues)
    {
        return;
    }

    Logger::Log(LogLevel::Info, L"Attempting connection to RabbitMQ");

    loop = uvw::Loop::getDefault();
    connectHandle = loop->resource<uvw::TCPHandle>();

    connectHandle->once<uvw::ErrorEvent>(
        [this](const uvw::ErrorEvent& event, uvw::TCPHandle&)
        {
            Logger::Log(LogLevel::Err, std::format(L"Socket error: {}", StringUtils::stows(event.what())));
            FLHook::GetConfig().messageQueue.enableQueues = false;
            runner.request_stop(); // Terminate thread runner
            isInitalizing = false;
        });

    connectHandle->once<uvw::ConnectEvent>(
        [this](const uvw::ConnectEvent&, uvw::TCPHandle& tcp)
        {
            // Authenticate with the RabbitMQ cluster.
            connection = std::make_unique<AMQP::Connection>(this, AMQP::Login("guest", "guest"), "/");

            // Start reading from the socket.
            connectHandle->read();
        });

    connectHandle->on<uvw::DataEvent>([this](const uvw::DataEvent& event, const uvw::TCPHandle&) { connection->parse(event.data.get(), event.length); });

    connectHandle->connect("localhost", 5672);
    runner = std::jthread([this] { loop->run<uvw::Loop::Mode::DEFAULT>(); });

    while (isInitalizing)
    {
        // TODO: Add if trace mode log to console
        Sleep(1000);
    }
}

void MessageHandler::onData(AMQP::Connection* conn, const char* data, size_t size) { connectHandle->write((char*)data, size); }

void MessageHandler::onReady(AMQP::Connection* conn)
{
    Logger::Log(LogLevel::Info, L"Connected to RabbitMQ!");
    isInitalizing = false;

    channel = std::make_unique<AMQP::Channel>(conn);
}

void MessageHandler::onError(AMQP::Connection* conn, const char* message)
{
    isInitalizing = false;
    Logger::Log(LogLevel::Err, std::format(L"AMQP error: {}", StringUtils::stows(message)));
}

void MessageHandler::onClosed(AMQP::Connection* conn) { std::cout << "closed" << std::endl; }

MessageHandler::~MessageHandler() = default;

void MessageHandler::Subscribe(const std::wstring& queue, QueueOnData callback, std::optional<QueueOnFail> onFail)
{
    if (!FLHook::GetConfig().messageQueue.enableQueues)
    {
        return;
    }

    if (!onMessageCallbacks.contains(queue))
    {
        onMessageCallbacks[queue] = { callback };
        if (onFail.has_value())
        {
            onFailCallbacks[queue] = { onFail.value() };
        }
        else
        {
            onFailCallbacks[queue] = {};
        }

        channel->consume(StringUtils::wstos(queue))
            .onSuccess([queue]() { Logger::Log(LogLevel::Info, std::format(L"successfully subscribed to {}", queue)); })
            .onReceived(
                [this, queue](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered)
                {
                    const auto callbacks = onMessageCallbacks.find(queue);
                    for (const auto& cb : callbacks->second)
                    {
                        std::optional<yyjson_mut_doc*> responseBody;
                        if (cb(message, responseBody))
                        {
                            if (message.headers().contains("reply_to"))
                            {
                                std::stringstream ss;
                                message.headers()["reply_to"].output(ss);

                                if (responseBody.has_value())
                                {
                                    uint size;
                                    auto str = yyjson_mut_write(responseBody.value(), 0, &size);
                                    channel->publish("", ss.str(), std::string(str, size));
                                    free(str);
                                }
                                else
                                {
                                    channel->publish("", ss.str(), "{}");
                                }
                            }

                            channel->ack(deliveryTag);
                            return;
                        }
                    }

                    channel->reject(deliveryTag);
                })
            .onError(
                [this, queue](const char* msg)
                {
                    Logger::Log(LogLevel::Warn, std::format(L"connection terminated with {} - {}", queue, StringUtils::stows(std::string(msg))));
                    const auto callbacks = onFailCallbacks.find(queue);
                    for (const auto& cb : callbacks->second)
                    {
                        cb(msg);
                    }
                });

        return;
    }

    onMessageCallbacks[queue].emplace_back(callback);
    if (onFail.has_value())
    {
        onFailCallbacks[queue].emplace_back(onFail.value());
    }
}

void MessageHandler::DeclareQueue(const std::wstring& queue, const int flags) const
{
    if (!channel)
    {
        return;
    }

    channel->declareQueue(StringUtils::wstos(queue), flags);
}

void MessageHandler::DeclareExchange(const std::wstring& exchange, const AMQP::ExchangeType type, const int flags) const
{
    if (!channel)
    {
        return;
    }

    channel->declareExchange(StringUtils::wstos(exchange), type, flags);
}

void MessageHandler::Publish(const std::wstring& jsonData, const std::wstring& exchange, const std::wstring& queue) const
{
    if (!channel)
    {
        return;
    }

    channel->publish(StringUtils::wstos(exchange), StringUtils::wstos(queue), StringUtils::wstos(jsonData));
}
