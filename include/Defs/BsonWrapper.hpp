#pragma once
#include <bson/bson.h>

#include <API/FLHook/BsonHelper.hpp>

class BsonWrapper
{
        bson_t* bson = nullptr;

        void AssembleBson(uint8_t* bytes, size_t size)
        {
            bson = bson_new_from_data(bytes, size);

            if (!bson)
            {
                return;
            }

            size_t errorOffset;
            if (!bson_validate(bson, static_cast<bson_validate_flags_t>(BSON_VALIDATE_DOT_KEYS | BSON_VALIDATE_UTF8 | BSON_VALIDATE_EMPTY_KEYS |
                                                                  BSON_VALIDATE_UTF8_ALLOW_NULL | BSON_VALIDATE_DOLLAR_KEYS), &errorOffset))
            {
                bson_destroy(bson);
                bson = nullptr;
            }
        }

    public:
        explicit BsonWrapper(const std::string_view bsonBytes)
        {
            const size_t bsonSize = bsonBytes.size();
            auto* bsonBuffer = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(bsonBytes.data()));
            AssembleBson(bsonBuffer, bsonSize);
        }

        explicit BsonWrapper(const B_VIEW bsonView)
        {
            auto* bsonData = const_cast<uint8_t*>(bsonView.data());
            const auto bsonSize = bsonView.length();
            AssembleBson(bsonData, bsonSize);
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
        std::optional<B_VIEW> GetValue() const
        {
            if (!bson)
            {
                return std::nullopt;
            }
            return { B_VIEW(bson_get_data(bson), bson->len) };
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

        static std::shared_ptr<BsonWrapper> CreateErrorDocument(const std::vector<std::string>& errors)
        {
            auto builder = B_ARR{};
            for (const auto& error : errors) {
                builder.append(error);
            }

            auto document = B_MDOC(B_KVP("errors", builder.view()));
            return std::make_shared<BsonWrapper>(document);
        }
};
