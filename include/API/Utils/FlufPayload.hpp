#pragma once

#include <rfl/msgpack.hpp>
#include <zstd.h>

constexpr USHORT flufHeader = 0xF10F;
struct DLL FlufPayload
{
        bool compressed{};
        char header[4]{};
        std::vector<char> data;

        template <typename T>
        rfl::Result<T> Convert() const
        {
            if (!compressed)
            {
                return rfl::msgpack::read<T>(data.data(), data.size());
            }

            const auto compressedSize = ZSTD_getFrameContentSize(data.data(), data.size());
            std::vector<char> uncompressedData(compressedSize);
            ZSTD_decompress(uncompressedData.data(), compressedSize, data.data(), data.size());

            return rfl::msgpack::read<T>(uncompressedData.data(), uncompressedData.size());
        }

        [[nodiscard]]
        std::vector<char> ToBytes() const
        {
            std::vector<char> bytes;
            bytes.resize(sizeof(flufHeader) + sizeof(header) + sizeof(compressed) + data.size());
            memcpy_s(bytes.data(), bytes.size(), &flufHeader, sizeof(flufHeader));
            memcpy_s(bytes.data() + sizeof(flufHeader), bytes.size() - sizeof(flufHeader), header, sizeof(header));
            memcpy_s(bytes.data() + sizeof(flufHeader) + sizeof(header), bytes.size() - sizeof(flufHeader) - sizeof(header), &compressed, sizeof(bool));
            memcpy_s(bytes.data() + sizeof(flufHeader) + sizeof(header) + sizeof(compressed),
                     bytes.size() - sizeof(flufHeader) - sizeof(header) - sizeof(compressed),
                     data.data(),
                     data.size());

            return bytes;
        }

        static std::optional<FlufPayload> FromPayload(char* data, size_t size)
        {
            // Check if enough room for the fluf header and the body, and that the header matches
            if (size < sizeof(flufHeader) + sizeof(header) + sizeof(compressed) + 1 || *reinterpret_cast<ushort*>(data) != flufHeader)
            {
                return std::nullopt;
            }

            FlufPayload payload;
            memcpy_s(payload.header, sizeof(payload.header), data + sizeof(flufHeader), sizeof(payload.header));
            memcpy_s(&payload.compressed, sizeof(payload.compressed), data + sizeof(flufHeader) + sizeof(header), sizeof(payload.compressed));
            const size_t newSize = size - sizeof(flufHeader) + sizeof(header) + sizeof(compressed);
            payload.data.resize(newSize);
            memcpy_s(payload.data.data(), newSize, data + sizeof(flufHeader) + sizeof(header) + sizeof(compressed), newSize);

            return payload;
        }

        template <typename T>
        static FlufPayload ToPayload(const T& data, const char header[4])
        {
            FlufPayload payload;
            memcpy_s(payload.header, sizeof(payload.header), header, sizeof(header));

            auto msgPack = rfl::msgpack::write(data);

            const size_t maxPossibleSize = ZSTD_compressBound(msgPack.size());
            payload.data.resize(maxPossibleSize);
            const size_t newSize = ZSTD_compress(payload.data.data(), maxPossibleSize, msgPack.data(), msgPack.size(), 6);

            if (newSize > msgPack.size())
            {
                payload.data = msgPack;
                payload.compressed = false;
                return payload;
            }

            payload.compressed = true;
            payload.data.resize(newSize); // Cut down to exact size
            return payload;
        }

    private:
        FlufPayload() = default;
};
