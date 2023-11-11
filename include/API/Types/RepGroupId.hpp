#pragma once

class RepGroupId final
{
        uint value = 0;

    public:
        explicit RepGroupId(const uint val) : value(val) {}
        explicit RepGroupId() = default;
        ~RepGroupId() = default;
        RepGroupId(const RepGroupId&) = default;
        RepGroupId& operator=(RepGroupId) = delete;
        RepGroupId(RepGroupId&&) = default;
        RepGroupId& operator=(RepGroupId&&) = delete;

        bool operator==(const RepGroupId& next) const { return value == next.value; }
        explicit operator bool() const { return value != 0; };

        uint GetValue() const { return value; }

        Action<std::wstring_view, Error> GetName() const;
        Action<std::wstring_view, Error> GetShortName() const;
};
