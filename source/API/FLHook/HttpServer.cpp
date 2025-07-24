#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/HttpServer.hpp"
#include "API/InternalApi.hpp"

#include <Psapi.h>
#include <bsoncxx/json.hpp>





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
    server->Get("/onlineplayers",
                [&](const httplib::Request& req, httplib::Response& res)
                {
                    std::scoped_lock lock(*this);
                    return GetOnlinePlayers(req, res);
                });

    server->Get("/ping",
                [&](const httplib::Request& req, httplib::Response& res)
                {
                    std::scoped_lock lock(*this);
                    return Ping();
                });

    CallPlugins(&Plugin::OnHttpServerRegister, server);

    // Start the server
    INFO("Running http server {{port}}", { "port", FLHook::GetConfig()->httpSettings.port });
    serverThread = std::jthread{ std::bind_front(&HttpServer::StartServer, this) };
    server->wait_until_ready();
    DEBUG("Http server started");
}

httplib::StatusCode HttpServer::GetOnlinePlayers(const httplib::Request& req, httplib::Response& res)
{
    PROCESS_MEMORY_COUNTERS memCounter;
    GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof memCounter);

    B_ARR players;
    for (auto& client : FLHook::Clients())
    {
        if (client.characterName.empty())
        {
            continue;
        }

        auto system = client.id.GetSystemId().Unwrap();

        // clang-format off
        players.append(B_MDOC(
            B_KVP("clientId", static_cast<int>(client.id.GetValue())),
            B_KVP("playerName", StringUtils::wstos(client.characterName)),
            B_KVP("systemName", system.GetValue() ? StringUtils::wstos(system.GetName().Unwrap()) : ""),
            B_KVP("systemNick", system.GetValue() ? StringUtils::wstos(system.GetNickName().Unwrap()) : ""),
            B_KVP("ipAddress", StringUtils::wstos(client.id.GetPlayerIp().Unwrap()))
        ));
    }

    const auto payload = B_MDOC(
        B_KVP("players", players),
        B_KVP("memUsage", static_cast<int64_t>(memCounter.WorkingSetSize)),
        B_KVP("npcsEnabled", InternalApi::NpcsEnabled())
    );

    // clang-format on

    WriteHttpResponse(req, payload, res);

    return httplib::StatusCode::OK_200;
}

void HttpServer::WriteHttpResponse(const httplib::Request& request, bsoncxx::v_noabi::document::value payload, httplib::Response& response)
{
    std::vector<std::string> accepts;
    for (auto accept = request.headers.find("Accept"); accept != request.headers.end(); accept++)
    {
        accepts.emplace_back(accept->second);
    }

    if (std::ranges::find(accepts, "application/bson") != accepts.end())
    {
        const std::string bytes = { reinterpret_cast<const char*>(payload.data()), payload.length() };
        response.set_content(bytes, "application/bson");
    }
    else
    {
        std::string bytes = bsoncxx::to_json(payload.view());
        response.set_content(bytes, "application/json");
    }
}

httplib::StatusCode HttpServer::Ping() { return httplib::StatusCode::OK_200; }

void HttpServer::lock() { mutex.lock(); }
void HttpServer::unlock() { mutex.unlock(); }
bool HttpServer::try_lock() { return mutex.try_lock(); }

HttpServer::~HttpServer()
{
    server->stop();
    serverThread.join();
}

HttpServer::HttpServer() { server = std::make_unique<httplib::Server>(); }
