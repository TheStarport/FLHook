#pragma once

#pragma warning(push, 0)
#pragma warning(disable : 4244)
#include "Defs/BsonWrapper.hpp"

#include <amqpcpp.h>
#include <memory>
#include <thread>
#include <uvw.hpp>
#include <vector>
#pragma warning(pop)

class MessageHandler final : public AMQP::ConnectionHandler, public Singleton<MessageHandler>
{
        using QueueOnData = std::function<bool(const AMQP::Message& msg, std::shared_ptr<BsonWrapper>& replyBody)>;
        using QueueOnFail = std::function<void(const char* err)>;

        std::shared_ptr<uvw::loop> loop;
        std::shared_ptr<uvw::tcp_handle> connectHandle;
        std::unique_ptr<AMQP::Connection> connection;
        std::unique_ptr<AMQP::Channel> channel;
        std::unordered_map<std::wstring, std::vector<QueueOnData>> onMessageCallbacks;
        std::unordered_map<std::wstring, std::vector<QueueOnFail>> onFailCallbacks;
        std::atomic_bool isInitalizing = true;
        std::jthread runner;

        void onData(AMQP::Connection* connection, const char* data, size_t size) override;
        void onReady(AMQP::Connection* connection) override;
        void onError(AMQP::Connection* connection, const char* message) override;
        void onClosed(AMQP::Connection* connection) override;

    public:
        explicit MessageHandler();
        ~MessageHandler() override;

        enum class Queue
        {
            ServerStats,
            ExternalCommands,
        };

        static std::wstring_view QueueToStr(const Queue queue) { return magic_enum::enum_name<Queue>(queue); }
        void Subscribe(const std::wstring& queue, QueueOnData callback, std::optional<QueueOnFail> onFail = std::nullopt);
        void Publish(const std::wstring& jsonData, const std::wstring& exchange = L"", const std::wstring& queue = L"") const;
        void DeclareQueue(const std::wstring& queue, int flags = 0) const;
        void DeclareExchange(const std::wstring& exchange, AMQP::ExchangeType type = AMQP::fanout, int flags = 0) const;
};
