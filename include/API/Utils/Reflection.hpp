#pragma once

#include "API/InternalApi.hpp"
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

    struct RepGroupImpl
    {
            using ReflectionType = std::string;
            std::string data;
            static RepGroupImpl from_class(const RepGroupId& hash) noexcept { return { "li_n_grp" }; }

            [[nodiscard]]
            RepGroupId to_class() const
            {
                return RepGroupId{ static_cast<uint>(MakeId(data.c_str())) };
            }
    };
    template <class ReaderType, class WriterType, class ProcessorsType>
    struct Parser<ReaderType, WriterType, RepGroupId, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, RepGroupId, RepGroupImpl>
    {};

    struct EquipmentId_Impl
    {
            using ReflectionType = std::string;
            std::string data;
            static EquipmentId_Impl from_class(const EquipmentId& hash) noexcept
            {
                const auto lookup = InternalApi::HashLookup(hash.GetId());
                return { lookup.empty() ? "nickname" : lookup };
            }
            [[nodiscard]]
            EquipmentId to_class() const
            {
                return EquipmentId{ CreateID(data.c_str()) };
            }
    };
    template <class ReaderType, class WriterType, class ProcessorsType>
    struct Parser<ReaderType, WriterType, EquipmentId, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, EquipmentId, EquipmentId_Impl>
    {};
    ;
    struct BaseId_Impl
    {
            using ReflectionType = std::string;
            std::string data;
            static BaseId_Impl from_class(const BaseId& hash) noexcept
            {
                const auto lookup = InternalApi::HashLookup(hash.GetValue());
                return { lookup.empty() ? "li01_01_base" : lookup };
            }
            [[nodiscard]]
            BaseId to_class() const
            {
                return BaseId{ CreateID(data.c_str()) };
            }
    };
    template <class ReaderType, class WriterType, class ProcessorsType>
    struct Parser<ReaderType, WriterType, BaseId, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, BaseId, BaseId_Impl>
    {};
    ;
    struct SystemId_Impl
    {
            using ReflectionType = std::string;
            std::string data;
            static SystemId_Impl from_class(const SystemId& hash) noexcept
            {
                const auto lookup = InternalApi::HashLookup(hash.GetValue());
                return { lookup.empty() ? "li01_01" : lookup };
            }
            [[nodiscard]]
            SystemId to_class() const
            {
                return SystemId{ CreateID(data.c_str()) };
            }
    };
    template <class ReaderType, class WriterType, class ProcessorsType>
    struct Parser<ReaderType, WriterType, SystemId, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, SystemId, SystemId_Impl>
    {};
    ;
    struct GoodId_Impl
    {
            using ReflectionType = std::string;
            std::string data;
            static GoodId_Impl from_class(const GoodId& hash) noexcept
            {
                if(!hash.GetValue())
                {
                    return {"commodity_cardamine"};
                }
                const auto lookup = InternalApi::HashLookup(hash.GetValue()->goodId);
                return { lookup.empty() ? "commodity_cardamine" : lookup };
            }
            [[nodiscard]]
            GoodId to_class() const
            {
                return GoodId{ CreateID(data.c_str()) };
            }
    };
    template <class ReaderType, class WriterType, class ProcessorsType>
    struct Parser<ReaderType, WriterType, GoodId, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, GoodId, GoodId_Impl>
    {};
    ;
    struct Id_Impl
    {
            using ReflectionType = std::string;
            std::string data;
            static Id_Impl from_class(const Id& hash) noexcept
            {
                const auto lookup = InternalApi::HashLookup(hash.GetValue());
                return { lookup.empty() ? "nickname" : lookup };
            }
            [[nodiscard]]
            Id to_class() const
            {
                return Id{ CreateID(data.c_str()) };
            }
    };
    template <class ReaderType, class WriterType, class ProcessorsType>
    struct Parser<ReaderType, WriterType, Id, ProcessorsType> : CustomParser<ReaderType, WriterType, ProcessorsType, Id, Id_Impl>
    {};

} // namespace rfl::parsing
