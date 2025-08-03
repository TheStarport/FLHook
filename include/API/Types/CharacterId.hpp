#pragma once

class DLL CharacterId final
{
        std::wstring characterName;

        static Action<bsoncxx::document::value> GetCharacterDocument(std::string_view name);

    public:
        explicit CharacterId(std::wstring_view characterName);
        explicit CharacterId() = default;

        bool operator==(const CharacterId& acc) const { return acc.characterName == characterName; }
        explicit operator bool() const;

        [[nodiscard]]
        std::wstring_view GetValue() const;

        [[nodiscard]]
        ClientData* GetOnlineData() const;

        concurrencpp::result<Action<void>> Delete() const;
        concurrencpp::result<Action<void>> Transfer(const AccountId& account) const;
        concurrencpp::result<Action<void>> SetTransferCode(std::wstring_view code) const;
        concurrencpp::result<Action<void>> Rename(std::wstring_view name) const;
        concurrencpp::result<Action<void>> AdjustCash(int cash) const;
        concurrencpp::result<Action<void>> AddCash(int cash) const;
        concurrencpp::result<Action<void>> RemoveCash(int cash) const;
        concurrencpp::result<Action<void>> AddCargo(GoodId good, uint count = 1) const;
        concurrencpp::result<Action<void>> RemoveCargo(GoodId good, uint count = 1) const;
        concurrencpp::result<Action<void>> SetPosition(Vector pos) const;
        concurrencpp::result<Action<void>> SetSystem(SystemId system) const;
        concurrencpp::result<Action<void>> Undock(Vector pos, SystemId system = {}, Matrix orient = Matrix::Identity()) const;
        concurrencpp::result<Action<void>> SetCharacterValue(std::string_view key, bsoncxx::document::value value) const;
        concurrencpp::result<Action<int>> GetCash() const;
        concurrencpp::result<Action<SystemId>> GetSystem() const;
        concurrencpp::result<Action<RepGroupId>> GetAffiliation() const;
        concurrencpp::result<Action<Vector>> GetPosition() const;
        concurrencpp::result<Action<bsoncxx::document::value>> GetCharacterData() const;
};
