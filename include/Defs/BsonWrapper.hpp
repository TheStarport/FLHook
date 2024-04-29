#pragma once
#include <bson/bson-memory.h>
#include <bson/bson.h>
#include <bsoncxx/document/view.hpp>

class BsonWrapper
{
    bson_t* bson;
    public:
    explicit BsonWrapper(std::string_view bsonBytes)
    {
        size_t bsonSize = bsonBytes.size();
        auto* bsonBuffer = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(bsonBytes.data()));
        bson = bson_new_from_buffer(&bsonBuffer, &bsonSize, bson_realloc_ctx, nullptr);
    }
    ~BsonWrapper()
    {
        bson_destroy(bson);
    }

    [[nodiscard]] std::optional<bsoncxx::document::view> GetValue() const
    {
        if(!bson)
        {
            return std::nullopt;
        }
        return {bsoncxx::document::view(bson_get_data(bson), bson->len)};
    }
};