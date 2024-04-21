#pragma once

class DLL AccountId final
{
        std::string accountId = "";

        struct ClientData* IsOnline() const;

    public:
        explicit AccountId() = default;

        static std::optional<AccountId> GetAccountFromClient(ClientId client);
        static std::optional<AccountId> GetAccountFromCharacterName(std::wstring_view characterName);
        static std::optional<AccountId> GetAccountFromAccountId(std::wstring_view accountId);

        bool operator==(AccountId acc) const { return accountId == acc.accountId; }
        explicit operator bool() const;

        [[nodiscard]]
        std::string_view GetValue() const;

        bool IsAdmin() const;

        Action<void, Error> UnBan() const;
        Action<void, Error> Ban(uint tempBanDays = 0) const;
        Action<void, Error> DeleteCharacter(std::wstring_view name) const;

        Action<void, Error> AddRoles(const std::vector<std::wstring_view>& roles);
        Action<void, Error> RemoveRoles(const std::vector<std::wstring_view>& roles, bool clear);
        Action<void, Error> SetRoles(const std::vector<std::wstring_view>& roles);

        Action<void, Error> SetCash(std::wstring_view characterName, int64 amount) const;
};
