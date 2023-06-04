#pragma once

#pragma warning(push, 0)
#pragma warning(disable: 4244)
#include <amqpcpp.h>
#include <ext/uvw.hpp>
#include <memory>
#include <vector>
#include <ext/Singleton.h>
#include <thread>
#pragma warning(pop)

class MessageHandler final : public AMQP::ConnectionHandler, public Singleton<MessageHandler>
{
	using QueueOnData = std::function<bool(const AMQP::Message& msg)>;
	using QueueOnFail = std::function<void(const char* err)>;

	std::shared_ptr<uvw::Loop> loop;
	std::shared_ptr<uvw::TCPHandle> connectHandle;
	std::unique_ptr<AMQP::Connection> connection;
	std::unique_ptr<AMQP::Channel> channel;
	std::map<std::wstring, std::vector<QueueOnData>> onMessageCallbacks;
	std::map<std::wstring, std::vector<QueueOnFail>> onFailCallbacks;
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
		ServerStats,
	};

	static std::wstring_view QueueToStr(const Queue queue) { return magic_enum::enum_name<Queue>(queue); }
	void Subscribe(const std::wstring& queue, QueueOnData callback, std::optional<QueueOnFail> onFail = std::nullopt);
	void Publish(const std::wstring& jsonData, const std::wstring& exchange = L"", const std::wstring& queue = L"") const;
	void DeclareQueue(const std::wstring& queue, int flags = 0) const;
	void DeclareExchange(const std::wstring& exchange, AMQP::ExchangeType type = AMQP::fanout, int flags = 0) const;
};
