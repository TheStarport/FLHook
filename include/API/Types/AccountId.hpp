#pragma once

class AccountId final
{
        CAccount* value = nullptr;

    public:
        explicit AccountId(ClientId client);
        explicit AccountId(std::wstring_view characterName);
        explicit AccountId() = default;

        bool operator==(const CAccount* next) const { return value == next; }
        explicit operator bool() const;

        [[nodiscard]]
        CAccount* GetValue() const
        {
            return value;
        }
};
