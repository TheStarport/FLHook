#pragma once

#include <httplib.h>

class DLL HttpServer
{
        friend FLHook;
        friend IServerImplHook;

        std::mutex mutex;
        std::shared_ptr<httplib::Server> server;
        std::jthread serverThread;

        void StartServer() const;
        void RegisterRoutes();
        httplib::StatusCode GetOnlinePlayers(const httplib::Request& req, httplib::Response& res);
		httplib::StatusCode Ping();

    public:
        void lock();
        void unlock();
        bool try_lock();
        ~HttpServer();
        HttpServer();

        void static WriteHttpResponse(const httplib::Request& request, bsoncxx::v_noabi::document::value payload, httplib::Response& response);
};
