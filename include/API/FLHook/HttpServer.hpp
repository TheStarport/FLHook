#pragma once

#include <httplib.h>

class HttpServer
{
        friend FLHook;
        friend IServerImplHook;

        std::mutex mutex;
        std::unique_ptr<httplib::Server> server;
        std::jthread serverThread;

        void StartServer() const;
        void RegisterRoutes();

    public:
        void lock();
        void unlock();
        bool try_lock();
        ~HttpServer();
        HttpServer();
};
