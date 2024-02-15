#pragma once

#include "Character.hpp"

struct Account
{
        std::string _id;
        std::vector<Character> characters;
        bool banned;
        int64 scheduledUnbanDate;
        int64 cash;
        std::optional<std::vector<std::string>> gameRoles;
        std::optional<std::vector<std::string>> webRoles;
        std::optional<std::string> hashedToken;
        std::optional<std::vector<byte>> salt;
        std::string username;
        std::string passwordHash;
};
