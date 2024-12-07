#include "PCH.hpp"

#include "API/FLHook/HttpServer.hpp"

#include "API/InternalApi.hpp"

#include <Psapi.h>
#include <bsoncxx/json.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

void HttpServer::StartServer() const
{
    const auto& config = FLHook::GetConfig();
    server->set_read_timeout(config->httpSettings.timeout);
    server->set_write_timeout(config->httpSettings.timeout);
    server->set_payload_max_length(config->httpSettings.maxPayloadSize);
    server->listen(config->httpSettings.host, config->httpSettings.port);
}

void HttpServer::RegisterRoutes()
{
    server->Get("/onlineplayers", [&](const httplib::Request& req, httplib::Response& res) { return GetOnlinePlayers(req, res); });

    // Start the server
    Logger::Info(std::format(L"Running http server on port {}", FLHook::GetConfig()->httpSettings.port));
    serverThread = std::jthread{ std::bind_front(&HttpServer::StartServer, this) };
    server->wait_until_ready();
    Logger::Debug(L"Http server started");
}

httplib::StatusCode HttpServer::GetOnlinePlayers(const httplib::Request& req, httplib::Response& res)
{
    std::scoped_lock lock(*this);

    PROCESS_MEMORY_COUNTERS memCounter;
    GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof memCounter);

    bsoncxx::builder::basic::array players;
    for (auto& client : FLHook::Clients())
    {
        if (client.characterName.empty())
        {
            continue;
        }

        auto system = client.id.GetSystemId().Unwrap();

        // clang-format off
        players.append(make_document(
            kvp("clientId", client.id.GetValue()),
            kvp("playerName", StringUtils::wstos(client.characterName)),
            kvp("systemName", system.GetValue() ? StringUtils::wstos(system.GetName().Unwrap()) : ""),
            kvp("systemNick", system.GetValue() ? StringUtils::wstos(system.GetNickName().Unwrap()) : ""),
            kvp("ipAddress", StringUtils::wstos(client.id.GetPlayerIp().Unwrap()))
        ));
    }

    const auto payload = make_document(
        kvp("players", players),
        kvp("memUsage", memCounter.WorkingSetSize),
        kvp("npcsEnabled", InternalApi::NpcsEnabled())
    );

    // clang-format on

    if (const auto config = FLHook::GetConfig(); config->httpSettings.sendJsonInsteadOfBson)
    {
        std::string bytes = bsoncxx::to_json(payload.view());
        res.set_content(bytes, "application/json");
    }
    else
    {
        const std::string bytes = { reinterpret_cast<const char*>(payload.data()), payload.length() };
        res.set_content(bytes, "application/bson");
    }

    return httplib::StatusCode::OK_200;
}

void HttpServer::lock() { mutex.lock(); }
void HttpServer::unlock() { mutex.unlock(); }
bool HttpServer::try_lock() { return mutex.try_lock(); }

HttpServer::~HttpServer()
{
    server->stop();
    serverThread.join();
}

HttpServer::HttpServer() { server = std::make_unique<httplib::Server>(); }
