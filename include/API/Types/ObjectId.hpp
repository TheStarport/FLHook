#pragma once

class ObjectId
{
        const uint value;

    public:
        explicit ObjectId(const uint val) : value(val) {}
        explicit operator uint() const noexcept { return value; }
        bool operator==(const ObjectId next) const { return value == next.value; }
        bool operator!() const;

        ObjectType GetObjectType();
        std::wstring GetNickName();

};