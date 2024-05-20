#pragma once

#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>

struct MailRecipient
{
    bsoncxx::oid target;
    std::optional<bsoncxx::types::b_date> readDate;
};

struct Mail
{
    bsoncxx::oid _id;
    std::optional<bsoncxx::oid> author;
    std::optional<std::string> origin;
    bsoncxx::types::b_date sentDate{ static_cast<std::chrono::milliseconds>(0) };
    std::string message;
    std::vector<MailRecipient> recipients;
};