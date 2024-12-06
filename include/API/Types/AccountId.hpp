#pragma once

class DLL AccountId final
{
        std::string accountId;

        [[nodiscard]]
        struct ClientData* IsOnline() const;

    public:
        explicit AccountId() = default;

        static std::optional<AccountId> GetAccountFromClient(ClientId client);
        static std::optional<AccountId> GetAccountFromCharacterName(std::wstring_view characterName);
        static std::optional<AccountId> GetAccountFromAccountId(std::wstring_view accountId);

        bool operator==(const AccountId& acc) const { return accountId == acc.accountId; }
        explicit operator bool() const;

        [[nodiscard]]
        std::string_view GetValue() const;

        [[nodiscard]]
        bool IsAdmin() const;

        Action<void> UnBan() const;
        Action<void> Ban(uint tempBanDays = 0) const;
        Action<void> DeleteCharacter(std::wstring_view name) const;

        Action<bool> HasRole(std::wstring_view role) const;
};
