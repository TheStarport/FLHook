#include "Global.hpp"
#include <mqtt/async_client.h>

std::tuple<mqtt::async_client_ptr, mqtt::connect_options_ptr, mqtt::callback_ptr> ConnectBroker()
{
	const auto config = FLHookConfig::c();
	auto connOpts = std::make_shared<mqtt::connect_options>();
	connOpts->set_automatic_reconnect(5, 120);
	auto client = std::make_shared<mqtt::async_client>("tcp://localhost:1883", "FLServer");
	auto cb = std::make_shared<callback>();
	client->set_callback(*cb);

	try
	{
		ConPrint(L"MQTT: connecting...\n");
		mqtt::token_ptr conntok = client->connect(*connOpts);

		// Wait for connection to be established TODO: This just waits if the
		// mqtt server is down and lags the server
		ConPrint(L"MQTT: Waiting for connection...\n");
		conntok->wait();
		ConPrint(L"MQTT: Connected\n");

		// Subscribe to /allplayers
		const std::string TOPIC("allplayers");
		client->subscribe(TOPIC, 0);
	}
	catch (const mqtt::exception& exc)
	{
		ConPrint(L"MQTT: error " + stows(exc.what()) + L"\n");
	}
	return std::make_tuple(client, connOpts, cb);
}

void SetupMessageQueue()
{
}