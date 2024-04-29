#pragma once
#include <bson/bson.h>
#include <bsoncxx/document/view.hpp>

class BsonWrapper
{
        bson_t* bson = nullptr;

    public:
        explicit BsonWrapper(const std::string_view bsonBytes)
        {
            size_t bsonSize = bsonBytes.size();
            auto* bsonBuffer = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(bsonBytes.data()));
            bson = bson_new_from_buffer(&bsonBuffer, &bsonSize, bson_realloc_ctx, nullptr);
        }

        explicit BsonWrapper(const bsoncxx::document::view bsonView)
        {
            auto* bsonData = const_cast<uint8_t*>(bsonView.data());
            auto bsonSize = bsonView.length();
            bson = bson_new_from_buffer(&bsonData, &bsonSize, bson_realloc_ctx, nullptr);
        }

        BsonWrapper() = default;

        ~BsonWrapper()
        {
            if (bson)
            {
                bson_destroy(bson);
            }
        }

        [[nodiscard]]
        std::optional<bsoncxx::document::view> GetValue() const
        {
            if (!bson)
            {
                return std::nullopt;
            }
            return { bsoncxx::document::view(bson_get_data(bson), bson->len) };
        }

        [[nodiscard]]
        std::string_view GetBytes() const
        {
            if (!bson)
            {
                return {};
            }
            return { reinterpret_cast<const char*>(bson_get_data(bson)), bson->len };
        }
};
