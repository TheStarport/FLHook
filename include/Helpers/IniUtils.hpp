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
		void CharacterClearClientInfo(ClientId client);
		void CharacterSelect(CHARACTER_ID charId, ClientId client);
#endif

		cpp::result<std::wstring, Error> GetFromPlayerFile(const std::variant<uint, std::wstring_view>& player, const std::wstring& key) const;
		Action<void> WriteToPlayerFile(const std::variant<uint, std::wstring_view>& player, const std::wstring& key, const std::wstring& value);

		void SetCharacterIni(ClientId client, const std::wstring& name, std::wstring value);
		std::wstring GetCharacterIniString(ClientId client, const std::wstring& name);
		bool GetCharacterIniBool(ClientId client, const std::wstring& name);
		int GetCharacterIniInt(ClientId client, const std::wstring& name);
		uint GetCharacterIniUint(ClientId client, const std::wstring& name);
		float GetCharacterIniFloat(ClientId client, const std::wstring& name);
		double GetCharacterIniDouble(ClientId client, const std::wstring& name);
		int64_t GetCharacterIniInt64(ClientId client, const std::wstring& name);
	};

} // namespace Hk