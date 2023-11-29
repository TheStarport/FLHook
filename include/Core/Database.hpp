#pragma once

#include <string>


#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>


class Database
{
      
        Database(const std::string& uri);
        ~Database();


        mongocxx::instance instance;
        mongocxx::collection accounts;

};