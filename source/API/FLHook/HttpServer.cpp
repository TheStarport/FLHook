#include "PCH.hpp"

#include "API/FLHook/HttpServer.hpp"

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
                [](const httplib::Request& req, httplib::Response& res)
                {
                    std::scoped_lock lock(mutex);

                    auto test = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("a", 1));
                    std::string bytes = { reinterpret_cast<const char*>(test.data()), test.length() };
                    res.set_content(bytes, "application/bson");
                    return httplib::StatusCode::OK_200;
                    //
                });

    // Start the server
    serverThread = std::jthread{ std::bind_front(&HttpServer::StartServer, this) };
}

HttpServer::~HttpServer()
{
    server->stop();
    serverThread.join();
}

HttpServer::HttpServer() { server = std::make_unique<httplib::Server>(); }
