#include "PCH.hpp"

#include "API/FLHook/Database.hpp"

using namespace sw::redis;

sw::redis::Redis Database::SetupConnection()
{
    try
    {
        const auto& db = FLHookConfig::i()->db;
        ConnectionOptions options;
        options.host = db.host;
        options.password = db.password;
        options.port = db.port;
        options.socket_timeout = std::chrono::milliseconds(db.timeout);

        ConnectionPoolOptions pool;
        pool.size = db.poolSize;
        pool.wait_timeout = options.socket_timeout;

        return Redis(options, pool);
    }
    catch (sw::redis::Error& err)
    {
        Logger::i()->Log(LogLevel::Err, std::format(L"Unable to create database connection.", StringUtils::stows(err.what())));
        throw err;
    }
}

Database::Database() {}
