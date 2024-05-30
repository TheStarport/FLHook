#pragma once

namespace AMQP
{
    class Message;
}

class MessageHandler final
{
        friend FLHook;

        static void PublishServerStats();
        static bool ProcessExternalCommand(const AMQP::Message& message, std::shared_ptr<BsonWrapper>& response);

    public:
        MessageHandler();
        MessageHandler(const MessageHandler&) = delete;
        MessageHandler(const MessageHandler&&) = delete;
        ~MessageHandler();
};
