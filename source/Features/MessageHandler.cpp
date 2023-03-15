#include "PCH.hpp"
#include <FLHook.hpp>
#include "Features/MessageHandler.hpp"

#include <magic_enum.hpp>

MessageHandler::MessageHandler()
{
	Logger::i()->Log(LogLevel::Info, "Attempting connection to RabbitMQ");

	loop = uvw::Loop::getDefault();
	connectHandle = loop->resource<uvw::TCPHandle>();

	connectHandle->once<uvw::ErrorEvent>([](const uvw::ErrorEvent& event, uvw::TCPHandle&) {
		// Just die on error, the message director always needs a connection to RabbitMQ.
		// TODO: Add proper error handling and reconnects in the event of connection loss
		Logger::i()->Log(LogLevel::Err, std::format("Socket error: {}", event.what()));
		throw std::runtime_error("Unable to connect to socket");
	});

	connectHandle->once<uvw::ConnectEvent>([this](const uvw::ConnectEvent&, uvw::TCPHandle& tcp) {
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

void MessageHandler::onData(AMQP::Connection* conn, const char* data, size_t size)
{
	connectHandle->write((char*)data, size);
}

void MessageHandler::onReady(AMQP::Connection* conn)
{
	Logger::i()->Log(LogLevel::Info, "Connected to RabbitMQ!");
	isInitalizing = false;

	channel = std::make_unique<AMQP::Channel>(conn);

	magic_enum::enum_for_each<Queue>([this](auto val) {
		constexpr Queue queue = val;
		this->channel->declareQueue(std::string(magic_enum::enum_name<Queue>(queue)), AMQP::durable);
	});
}

void MessageHandler::onError(AMQP::Connection* conn, const char* message)
{
	isInitalizing = false;
	Logger::i()->Log(LogLevel::Err, std::format("AMQP error: {}", message));
}

void MessageHandler::onClosed(AMQP::Connection* conn)
{
	std::cout << "closed" << std::endl;
}

MessageHandler::~MessageHandler() = default;

void MessageHandler::Subscribe(const std::string& queue, QueueOnData callback, std::optional<QueueOnFail> onFail)
{
	if (!onMessageCallbacks.contains(queue))
	{
		onMessageCallbacks[queue] = {callback};
		if (onFail.has_value())
		{
			onFailCallbacks[queue] = {onFail.value()};
		}
		else
		{
			onFailCallbacks[queue] = {};
		}

		channel->consume(queue)
		       .onSuccess([queue]() {
			       Logger::i()->Log(LogLevel::Info, std::format("successfulled subscribed to {}", queue));
		       })
		       .onReceived([this, queue](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered) {
			       const auto callbacks = onMessageCallbacks.find(queue);
			       for (const auto& cb : callbacks->second)
			       {
				       if (cb(message))
				       {
					       channel->ack(deliveryTag);
					       return;
				       }
			       }
		       })
		       .onError([this, queue](const char* msg) {
			       Logger::i()->Log(LogLevel::Warn, std::format("connection terminated with {} - {}", queue, std::string(msg)));
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

