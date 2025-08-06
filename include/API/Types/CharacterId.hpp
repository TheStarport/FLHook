#pragma once

#include "API/Utils/Action.hpp"

#include <concurrencpp/concurrencpp.h>
#include <API/FLHook/BsonHelper.hpp>

class AccountId;
class GoodId;
enum class MongoResult;
class RepGroupId;
struct ClientData;
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

        std::string GetCharacterCode() const;
        concurrencpp::result<Action<MongoResult>> Delete() const;
        concurrencpp::result<Action<MongoResult>> Transfer(AccountId targetAccount, std::wstring_view transferCode) const;
        concurrencpp::result<Action<MongoResult>> SetTransferCode(std::wstring_view code) const;
        concurrencpp::result<Action<MongoResult>> ClearTransferCode() const;
        concurrencpp::result<Action<MongoResult>> Rename(std::wstring_view name) const;
        concurrencpp::result<Action<MongoResult>> AdjustCash(int cash) const;
        concurrencpp::result<Action<MongoResult>> AddCash(int cash) const;
        concurrencpp::result<Action<MongoResult>> RemoveCash(int cash) const;
        concurrencpp::result<Action<MongoResult>> SetCash(int cash) const;
        concurrencpp::result<Action<MongoResult>> AddCargo(GoodId good, uint count = 1, float health = 1.0f, bool mission = true) const;
        concurrencpp::result<Action<MongoResult>> RemoveCargo(GoodId good, uint count = 1) const;
        concurrencpp::result<Action<MongoResult>> SetPosition(Vector pos) const;
        concurrencpp::result<Action<MongoResult>> SetSystem(SystemId system) const;
        concurrencpp::result<Action<MongoResult>> Undock(Vector pos, SystemId system = {}, Matrix orient = Matrix::Identity()) const;
        concurrencpp::result<Action<int>> GetCash() const;
        concurrencpp::result<Action<SystemId>> GetSystem() const;
        concurrencpp::result<Action<RepGroupId>> GetAffiliation() const;
        concurrencpp::result<Action<Vector>> GetPosition() const;
        concurrencpp::result<Action<bsoncxx::document::value>> GetCharacterData() const;

        static concurrencpp::result<bool> CharacterExists(std::wstring_view characterName);

        /**
         * @param name The character that you wish to update
         * @param updateDoc A view of the desired update to be made to the character
         * @note This function does not change the thread it is being executed on. Ensure that you are running with THREAD_BACKGROUND before calling
         * @code{.cpp}
         * THREAD_BACKGROUND;
         * // Change Jeff's current system to New York
         * const auto updateDoc = B_MDOC(B_KVP("$set",
         *      B_MDOC(
         *          B_KVP("system", static_cast<int>(CreateID("li01"))))
         *      ));
         * CharacterId::UpdateCharacterDocument("Jeff", updateDoc.view()).Handle();
         * @endcode
         * @returns On success : void
         * @returns On fail : Error code of CharacterNameNotFound
         */
        static Action<MongoResult> UpdateCharacterDocument(std::string_view name, bsoncxx::document::view updateDoc);
};

template <>
struct std::formatter<CharacterId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const CharacterId& name, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", name.GetValue()); }
};

template <>
struct std::hash<CharacterId>
{
        std::size_t operator()(const CharacterId& id) const noexcept { return std::hash<std::wstring>()(std::wstring(id.GetValue())); }
};
