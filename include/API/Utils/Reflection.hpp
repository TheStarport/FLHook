#pragma once

#include "FLCore/FLCoreDefs.hpp"
#include "rfl/parsing/CustomParser.hpp"

// Custom parsers for FL types
namespace rfl::parsing {
    struct VectorImpl
    {
        float arr[3];

        static VectorImpl from_class(const Vector& val) noexcept {
            return VectorImpl{{val.x, val.y, val.z}};
        }

        [[nodiscard]] Vector to_class() const { return Vector{arr[0], arr[1], arr[2]}; }
    };

    template <class ReaderType, class WriterType>
    struct Parser<ReaderType, WriterType, Vector>
        : CustomParser<ReaderType, WriterType, Vector, VectorImpl> {};

} // namespace rfl::parsing
