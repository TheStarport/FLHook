#pragma once

#include <rfl/msgpack.hpp>
#include <zstd.h>

constexpr USHORT flufHeader = 0xF10F;
struct DLL FlufPayload
{
        bool compressed{};
        std::string header;
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
            bytes.resize(sizeof(flufHeader) + 1 + header.size() + sizeof(compressed) + data.size());

            auto ptr = bytes.data();
            const auto size = sizeof(flufHeader);
            memcpy_s(ptr, bytes.size(), &flufHeader, size);

            // Write our string header
            ptr += sizeof(flufHeader);
            auto newSize = bytes.size() - size + 1;

            *ptr = static_cast<byte>(header.size());
            ++ptr;
            memcpy_s(ptr, newSize, header.data(), header.size());

            // Write compressed boolean
            newSize -= header.size();
            ptr += header.size();
            memcpy_s(ptr, newSize, &compressed, sizeof(bool));

            --newSize;
            ++ptr;
            memcpy_s(ptr, newSize, data.data(), data.size());

            return bytes;
        }

        static std::optional<FlufPayload> FromPayload(char* data, size_t size)
        {
            // Check if enough room for the fluf header and the body, and that the header matches
            if (size < sizeof(flufHeader) + sizeof(compressed) + 2 || *reinterpret_cast<ushort*>(data) != flufHeader)
            {
                return std::nullopt;
            }

            const char* ptr = data + sizeof(flufHeader);
            const auto headerSize = *ptr;

            if (sizeof(flufHeader) + sizeof(compressed) + 2 + headerSize < size)
            {
                return std::nullopt;
            }
            FlufPayload payload;
            payload.data.resize(headerSize);
            memcpy_s(payload.header.data(), headerSize, ptr, headerSize);
            ptr += headerSize;

            memcpy_s(&payload.compressed, sizeof(payload.compressed), ptr, sizeof(payload.compressed));
            ptr += sizeof(payload.compressed);

            const size_t newSize = size - (ptr - data);
            payload.data.resize(newSize);
            memcpy_s(payload.data.data(), newSize, ptr, newSize);

            return payload;
        }

        template <typename T>
        static FlufPayload ToPayload(const T& data, const std::string_view header)
        {
            if (header.size() > 255)
            {
                throw std::runtime_error("Header size cannot be bigger than one byte.");
            }

            FlufPayload payload;
            payload.header.resize(header.size());
            memcpy_s(payload.header.data(), payload.header.size(), header.data(), header.size());

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
