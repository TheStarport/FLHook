#pragma once

namespace Hk
{
    class DLL IniUtils final : public Singleton<IniUtils>
    {
            static std::wstring GetIniValue(const std::wstring& data, const std::wstring& section, const std::wstring& key);

        public:
            IniUtils();
            ~IniUtils();

// Only compile for core
#ifdef FLHOOK
            static void CharacterClearClientInfo(ClientId client);
            void CharacterSelect(CHARACTER_ID charId, ClientId client) const;
#endif

            static cpp::result<std::wstring, Error> GetFromFile(const std::wstring& filePath, const std::wstring& section, const std::wstring& key);
            cpp::result<std::wstring, Error> GetFromPlayerFile(const std::variant<uint, std::wstring_view>& player, const std::wstring& key) const;
            Action<void> WriteToPlayerFile(const std::variant<uint, std::wstring_view>& player, const std::wstring& key, const std::wstring& value) const;

            void SetCharacterIni(ClientId client, const std::wstring& name, std::wstring value) const;
            std::wstring GetCharacterIni(ClientId client, const std::wstring& name);

    };

} // namespace Hk
