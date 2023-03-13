#pragma once

#include <amqpcpp.h>
#include <ext/uvw.hpp>
#include <memory>
#include <vector>
#include <ext/Singleton.h>
#include <thread>

class MessageHandler final : public AMQP::ConnectionHandler, public Singleton<MessageHandler>
{
	using QueueOnData = std::function<bool(const AMQP::Message& msg)>;
	using QueueOnFail = std::function<void(const char* err)>;

	std::shared_ptr<uvw::Loop> loop;
	std::shared_ptr<uvw::TCPHandle> connectHandle;
	std::unique_ptr<AMQP::Connection> connection;
	std::unique_ptr<AMQP::Channel> channel;
	std::map<std::string, std::vector<QueueOnData>> onMessageCallbacks;
	std::map<std::string, std::vector<QueueOnFail>> onFailCallbacks;
	std::atomic_bool isInitalizing = true;
	std::jthread runner;

	void onData(AMQP::Connection* connection, const char* data, size_t size) override;
	void onReady(AMQP::Connection* connection) override;
	void onError(AMQP::Connection* connection, const char* message) override;
	void onClosed(AMQP::Connection* connection) override;

public:
	explicit MessageHandler();
	~MessageHandler() noexcept override;

	enum class Queue
	{
		Test
	};

	static std::string QueueToStr(Queue queue) { return std::string(magic_enum::enum_name<Queue>(queue)); }
	void Subscribe(const std::string& queue, QueueOnData callback, std::optional<QueueOnFail> onFail = std::nullopt);
};
