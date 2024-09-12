#pragma once

#include "FLCore/FLCoreDefs.hpp"
#include "rfl/parsing/CustomParser.hpp"

#include <bsoncxx/oid.hpp>

// Custom parsers for FL types
namespace rfl::parsing
{
    struct VectorImpl
    {
            using ReflectionType = std::array<float, 3>;
            std::array<float, 3> arr;

            static VectorImpl from_class(const Vector& val) noexcept
            {
                return VectorImpl{
                    { val.x, val.y, val.z }
                };
            }

            [[nodiscard]]
            Vector to_class() const
            {
                return Vector{ arr[0], arr[1], arr[2] };
            }
    };

    template <class ReaderType, class WriterType, class ProcessorsType>
    struct Parser<ReaderType, WriterType, Vector, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, Vector, VectorImpl>
    {};

    struct BsonOidImpl
    {
            using ReflectionType = std::array<uint8_t, 12>;
            std::array<uint8_t, 12> bytes;

            static BsonOidImpl from_class(const bsoncxx::oid& val) noexcept
            {
                BsonOidImpl impl{};
                memcpy(impl.bytes.data(), val.bytes(), impl.bytes.size());
                return impl;
            }

            [[nodiscard]]
            bsoncxx::oid to_class() const
            {
                return bsoncxx::oid{ reinterpret_cast<const char*>(bytes.data()), bsoncxx::oid::size() };
            }
    };

    template <class ReaderType, class WriterType, class ProcessorsType>
    struct Parser<ReaderType, WriterType, bsoncxx::oid, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, bsoncxx::oid, BsonOidImpl>
    {};

#define HashConvert(hashType, example)                                                                                                                    \
    struct hashType##_Impl                                                                                                                                \
    {                                                                                                                                                     \
            using ReflectionType = std::string;                                                                                                           \
            std::string data;                                                                                                                             \
            static hashType##_Impl from_class(const hashType##& hash) noexcept { return {##example }; }                                                   \
                                                                                                                                                          \
            [[nodiscard]]                                                                                                                                 \
            ##hashType to_class() const                                                                                                                   \
            {                                                                                                                                             \
                return hashType##{ CreateID(data.c_str()) };                                                                                              \
            }                                                                                                                                             \
    };                                                                                                                                                    \
    template <class ReaderType, class WriterType, class ProcessorsType>                                                                                   \
    struct Parser<ReaderType, WriterType, hashType##, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, hashType##, hashType##_Impl> \
    {};

    HashConvert(EquipmentId, "commodity_cardamine");
    HashConvert(BaseId, "li01_01_base");
    HashConvert(RepGroupId, "fc_z_grp");
    HashConvert(SystemId, "li01_01");
    HashConvert(GoodId, "commodity_cardamine");
    HashConvert(Id, "nickname")

} // namespace rfl::parsing
