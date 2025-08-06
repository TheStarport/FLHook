#pragma once

class CharacterId;
class DLL AccountId final
{
        std::string accountId;

        [[nodiscard]]
        struct ClientData* IsOnline() const;

    public:
        explicit AccountId() = default;

        static std::optional<AccountId> GetAccountFromClient(ClientId client);
        static concurrencpp::result<std::optional<AccountId>> GetAccountFromCharacterName(const CharacterId& characterName);
        static concurrencpp::result<std::optional<AccountId>> GetAccountFromAccountId(std::wstring_view accountId);

        bool operator==(const AccountId& acc) const { return accountId == acc.accountId; }
        explicit operator bool() const;

        [[nodiscard]]
        std::string_view GetValue() const;

        [[nodiscard]]
        bool IsAdmin() const;

        concurrencpp::result<Action<void>> UnBan() const;
        concurrencpp::result<Action<void>> Ban(uint tempBanDays = 0) const;
        concurrencpp::result<Action<void>> DeleteCharacter(std::wstring_view name) const;

        Action<bool> HasRole(std::wstring_view role) const;
};
