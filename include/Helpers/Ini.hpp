#pragma once

namespace Hk::Ini
{
	DLL cpp::result<std::wstring, Error> GetFromPlayerFile(const std::variant<uint, std::wstring>& player, const std::wstring& Key);
	DLL cpp::result<void, Error> WriteToPlayerFile(const std::variant<uint, std::wstring>& player, const std::wstring& Key, const std::wstring& Value);

	DLL void SetCharacterIni(ClientId client, const std::wstring& name, std::wstring value);
	DLL std::wstring GetCharacterIniString(ClientId client, const std::wstring& name);
	DLL bool GetCharacterIniBool(ClientId client, const std::wstring& name);
	DLL int GetCharacterIniInt(ClientId client, const std::wstring& name);
	DLL uint GetCharacterIniUint(ClientId client, const std::wstring& name);
	DLL float GetCharacterIniFloat(ClientId client, const std::wstring& name);
	DLL double GetCharacterIniDouble(ClientId client, const std::wstring& name);
	DLL int64_t GetCharacterIniInt64(ClientId client, const std::wstring& name);
}