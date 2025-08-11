#pragma once

#include "Character.hpp"

struct DbAccount
{
        std::string _id;
        std::vector<bsoncxx::oid> characters;
        std::optional<int64> scheduledUnbanDate = 0;
        int64 cash = 0;
        std::optional<std::vector<std::string>> gameRoles;
        std::optional<std::vector<std::string>> webRoles;
        std::optional<std::string> hashedToken;
        std::optional<std::vector<byte>> salt;
        std::optional<std::string> username;
        std::optional<std::string> passwordHash;

        B_VAL accountData = B_MDOC();

        DbAccount() = default;
        explicit DbAccount(B_VIEW view);
        void ToBson(B_DOC& document) const;
};
