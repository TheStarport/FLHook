#include "PCH.hpp"

#include "Core/Database.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::stream::document;

Database::Database(const std::string& uri)
{
    try
    {
        const auto mongoURI = mongocxx::uri{ uri };
        mongocxx::options::client clientOptions;
        //TODO: Why does this server_api object not exist? It is on both the spike and documentation. 
        const auto api = mongocxx::options::server_api{ mongocxx::options::server_api::version::k_version_1 };


    }
    catch (std::exception& err)
    {
        Logger::Log(LogLevel::Err, StringUtils::stows(std::string(err.what())));

    }

}
