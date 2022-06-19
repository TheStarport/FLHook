#include "Global.hpp"

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
int __stdcall HkCb_UpdateFile(char* filename, wchar_t* savetime, int b)
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

__declspec(naked) void HkCb_UpdateFileNaked()
{
	__asm {
        mov CurrPlayer, ecx
        jmp HkCb_UpdateFile
	}
}

void HkCharacterClearClientInfo(uint client)
{
	clients.erase(client);
}

void HkCharacterSelect(CHARACTER_ID const charId, uint client)
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
void HkCharacterInit()
{
	clients.clear();
	if (patched)
		return;

	PatchCallAddr((char*)hModServer, 0x6c547, (char*)HkCb_UpdateFileNaked);
	PatchCallAddr((char*)hModServer, 0x6c9cd, (char*)HkCb_UpdateFileNaked);

	patched = true;
}

void HkCharacterShutdown()
{
	if (!patched)
		return;

	BYTE patch[] = { 0xE8, 0x84, 0x07, 0x00, 0x00 };
	WriteProcMem((char*)hModServer + 0x6c547, patch, 5);

	BYTE patch2[] = { 0xE8, 0xFE, 0x2, 0x00, 0x00 };
	WriteProcMem((char*)hModServer + 0x6c9cd, patch2, 5);

	patched = false;
}

std::wstring HkGetCharacterIniString(uint client, const std::wstring& name)
{
	if (clients.find(client) == clients.end())
		return L"";

	if (!clients[client].charfilename.length())
		return L"";

	if (clients[client].lines.find(name) == clients[client].lines.end())
		return L"";

	auto line = clients[client].lines[name];
	if (line == L"yes")
		line = L"true";
	else if (line == L"no")
		line = L"false";

	return line;
}

void HkSetCharacterIni(uint client, const std::wstring& name, std::wstring value)
{
	if (value == L"yes")
		value = L"true";
	else if (value == L"no")
		value = L"false";

	clients[client].lines[name] = std::move(value);
}

bool HkGetCharacterIniBool(uint client, const std::wstring& name)
{
	const auto val = HkGetCharacterIniString(client, name);
	return val == L"true" || val == L"1";
}

int HkGetCharacterIniInt(uint client, const std::wstring& name)
{
	const auto val = HkGetCharacterIniString(client, name);
	return wcstol(val.c_str(), nullptr, 10);
}

uint HkGetCharacterIniUint(uint client, const std::wstring& name)
{
	const auto val = HkGetCharacterIniString(client, name);
	return wcstoul(val.c_str(), nullptr, 10);
}

float HkGetCharacterIniFloat(uint client, const std::wstring& name)
{
	const auto val = HkGetCharacterIniString(client, name);
	return wcstof(val.c_str(), nullptr);
}

double HkGetCharacterIniDouble(uint client, const std::wstring& name)
{
	const auto val = HkGetCharacterIniString(client, name);
	return wcstod(val.c_str(), nullptr);
}

int64_t HkGetCharacterIniInt64(uint client, const std::wstring& name)
{
	const auto val = HkGetCharacterIniString(client, name);
	return wcstoll(val.c_str(), nullptr, 10);
}