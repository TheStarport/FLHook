#pragma once

class SystemId;
class ObjectId
{
    protected:
        const uint value = 0;

    public:
        explicit ObjectId(const uint val) : value(val) {}
        explicit ObjectId() = default;
        virtual ~ObjectId() = default;
        ObjectId(const ObjectId&) = default;
        ObjectId& operator=(ObjectId) = default;
        ObjectId(ObjectId&&) = default;
        ObjectId& operator=(ObjectId&&) = delete;

        bool operator==(const ObjectId& next) const { return value == next.value; }
        explicit virtual operator bool() const;

        [[nodiscard]]
        CObject::Class GetObjectType() const;

        [[nodiscard]]
        std::wstring GetNickName() const;

        [[nodiscard]]
        std::wstring GetName() const;

        [[nodiscard]]
        Action<CObjPtr, Error> GetCObject(bool increment = false) const;

        [[nodiscard]]
        Archetype::Root* GetArchetype() const;

        [[nodiscard]]
        Vector GetVelocity() const;

        [[nodiscard]]
        Vector GetAngularVelocity() const;

        [[nodiscard]]
        Vector GetPosition() const;

        [[nodiscard]]
        Matrix GetOrientation() const;

        [[nodiscard]]
        SystemId GetSystem() const;
};
