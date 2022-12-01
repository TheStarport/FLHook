#include "Global.hpp"

namespace Hk::Ini
{
	struct FLHOOK_PLAYER_DATA
	{
		std::string charfilename;
		std::map<std::wstring, std::wstring> lines;
	};

	std::map<uint, FLHOOK_PLAYER_DATA> clients;

	std::string GetAccountDir(uint client)
	{
		static _GetFLName GetFLName = (_GetFLName)((char*)hModServer + 0x66370);
		char dirname[1024];
		GetFLName(dirname, Players[client].Account->wszAccID);
		return dirname;
	}

	std::string GetCharfilename(const std::wstring& charname)
	{
		static _GetFLName GetFLName = (_GetFLName)((char*)hModServer + 0x66370);
		char filename[1024];
		GetFLName(filename, charname.c_str());
		return filename;
	}

	static PlayerData* CurrPlayer;
	int __stdcall Cb_UpdateFile(char* filename, wchar_t* savetime, int b)
	{
		// Call the original save charfile function
		int retv;
		__asm {
        pushad
        mov ecx, [CurrPlayer]
        push b
        push savetime
        push filename
        mov eax, 0x6d4ccd0
        call eax
        mov retv, eax
        popad
		}

		// Readd the flhook section.
		if (retv)
		{
			uint client = CurrPlayer->iOnlineID;

			std::string path = scAcctPath + GetAccountDir(client) + "\\" + filename;
			FILE* file;
			fopen_s(&file, path.c_str(), "a");
			if (file)
			{
				fwprintf(file, L"[flhook]\n");
				for (auto& i : clients[client].lines)
					fwprintf(file, L"%s = %s\n", i.first.c_str(), i.second.c_str());
				fclose(file);
			}
		}

		return retv;
	}

	__declspec(naked) void Cb_UpdateFileNaked()
	{
		__asm {
        mov CurrPlayer, ecx
        jmp Cb_UpdateFile
		}
	}

	void CharacterClearClientInfo(uint client) { clients.erase(client); }

	void CharacterSelect(CHARACTER_ID const charId, uint client)
	{
		std::string path = scAcctPath + GetAccountDir(client) + "\\" + charId.szCharFilename;

		clients[client].charfilename = charId.szCharFilename;
		clients[client].lines.clear();

		// Read the flhook section so that we can rewrite after the save so that it isn't lost
		INI_Reader ini;
		if (ini.open(path.c_str(), false))
		{
			while (ini.read_header())
			{
				if (ini.is_header("flhook"))
				{
					std::wstring tag;
					while (ini.read_value())
					{
						clients[client].lines[stows(ini.get_name_ptr())] = stows(ini.get_value_string());
					}
				}
			}
			ini.close();
		}
	}

	static bool patched = false;
	void CharacterInit()
	{
		clients.clear();
		if (patched)
			return;

		PatchCallAddr((char*)hModServer, 0x6c547, (char*)Cb_UpdateFileNaked);
		PatchCallAddr((char*)hModServer, 0x6c9cd, (char*)Cb_UpdateFileNaked);

		patched = true;
	}

	void CharacterShutdown()
	{
		if (!patched)
			return;

		BYTE patch[] = {0xE8, 0x84, 0x07, 0x00, 0x00};
		WriteProcMem((char*)hModServer + 0x6c547, patch, 5);

		BYTE patch2[] = {0xE8, 0xFE, 0x2, 0x00, 0x00};
		WriteProcMem((char*)hModServer + 0x6c9cd, patch2, 5);

		patched = false;
	}

	cpp::result<std::wstring, Error> GetFromPlayerFile(const std::variant<uint, std::wstring>& player, const std::wstring& wscKey)
	{
		std::wstring ret;
		const auto client = Hk::Client::ExtractClientId(player);
		const auto acc = Hk::Client::GetAccountByClientID(client);
		auto dir = Hk::Client::GetAccountDirName(acc);

		auto file = Hk::Client::GetCharFileName(player);
		if (file.has_error())
		{
			return cpp::fail(file.error());
		}

		if (const std::string scCharFile = scAcctPath + wstos(dir) + "\\" + wstos(file.value()) + ".fl"; Hk::Client::IsEncoded(scCharFile))
		{
			const std::string scCharFileNew = scCharFile + ".ini";
			if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
				return cpp::fail(Error::CouldNotDecodeCharFile);

			ret = stows(IniGetS(scCharFileNew, "Player", wstos(wscKey), ""));
			DeleteFile(scCharFileNew.c_str());
		}
		else
		{
			ret = stows(IniGetS(scCharFile, "Player", wstos(wscKey), ""));
		}

		return ret;
	}

	cpp::result<void, Error> WriteToPlayerFile(const std::variant<uint, std::wstring>& player, const std::wstring& wscKey, const std::wstring& wscValue)
	{
		const auto client = Hk::Client::ExtractClientId(player);
		const auto acc = Hk::Client::GetAccountByClientID(client);
		auto dir = Hk::Client::GetAccountDirName(acc);

		auto file = Hk::Client::GetCharFileName(player);
		if (file.has_error())
		{
			return cpp::fail(file.error());
		}

		if (std::string scCharFile = scAcctPath + wstos(dir) + "\\" + wstos(file.value()) + ".fl"; Hk::Client::IsEncoded(scCharFile))
		{
			std::string scCharFileNew = scCharFile + ".ini";
			if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
				return cpp::fail(Error::CouldNotDecodeCharFile);

			IniWrite(scCharFileNew, "Player", wstos(wscKey), wstos(wscValue));

			// keep decoded
			DeleteFile(scCharFile.c_str());
			MoveFile(scCharFileNew.c_str(), scCharFile.c_str());
		}
		else
		{
			IniWrite(scCharFile, "Player", wstos(wscKey), wstos(wscValue));
		}

		return {};
	}

	std::wstring GetCharacterIniString(uint client, const std::wstring& name)
	{
		if (clients.find(client) == clients.end())
			return L"";

		if (!clients[client].charfilename.length())
			return L"";

		if (clients[client].lines.find(name) == clients[client].lines.end())
			return L"";

		auto line = clients[client].lines[name];
		return line;
	}

	void SetCharacterIni(uint client, const std::wstring& name, std::wstring value) { clients[client].lines[name] = std::move(value); }

	bool GetCharacterIniBool(uint client, const std::wstring& name)
	{
		const auto val = GetCharacterIniString(client, name);
		return val == L"true" || val == L"1";
	}

	int GetCharacterIniInt(uint client, const std::wstring& name)
	{
		const auto val = GetCharacterIniString(client, name);
		return wcstol(val.c_str(), nullptr, 10);
	}

	uint GetCharacterIniUint(uint client, const std::wstring& name)
	{
		const auto val = GetCharacterIniString(client, name);
		return wcstoul(val.c_str(), nullptr, 10);
	}

	float GetCharacterIniFloat(uint client, const std::wstring& name)
	{
		const auto val = GetCharacterIniString(client, name);
		return wcstof(val.c_str(), nullptr);
	}

	double GetCharacterIniDouble(uint client, const std::wstring& name)
	{
		const auto val = GetCharacterIniString(client, name);
		return wcstod(val.c_str(), nullptr);
	}

	int64_t GetCharacterIniInt64(uint client, const std::wstring& name)
	{
		const auto val = GetCharacterIniString(client, name);
		return wcstoll(val.c_str(), nullptr, 10);
	}
} // namespace Hk::Ini