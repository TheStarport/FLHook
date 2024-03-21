#pragma once

#include "Character.hpp"

struct Account
{
        std::string _id;
        std::vector<bson_oid_t> characters;
        bool banned = false;
        std::optional<int64> scheduledUnbanDate = 0;
        int64 cash = 0;
        std::optional<std::vector<std::string>> gameRoles;
        std::optional<std::vector<std::string>> webRoles;
        std::optional<std::string> hashedToken;
        std::optional<std::vector<byte>> salt;
        std::optional<std::string> username;
        std::optional<std::string> passwordHash;
};