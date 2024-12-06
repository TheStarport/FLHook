#pragma once

#include <httplib.h>

class HttpServer
{
        friend FLHook;
        inline static std::mutex mutex;
        std::unique_ptr<httplib::Server> server;
        std::jthread serverThread;

        void StartServer() const;
        void RegisterRoutes();

    public:
        ~HttpServer();
        HttpServer();
};
