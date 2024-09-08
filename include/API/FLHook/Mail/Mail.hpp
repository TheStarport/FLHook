#pragma once

#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>

struct MailRecipient
{
    rfl::Variant<bsoncxx::oid, std::string> target;
    std::optional<rfl::Timestamp<"%F %R">> readDate;
};

struct Mail
{
    bsoncxx::oid _id;
    std::optional<bsoncxx::oid> author;
    std::optional<std::string> origin;
    rfl::Timestamp<"%F %R"> sentDate;
    std::string message;
    std::vector<MailRecipient> recipients;
};