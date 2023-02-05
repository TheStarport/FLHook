#include "MemoryManager.hpp"

bool SaveGameDetour::GetUserDataPathDetour(char* retPtr)
{
	const auto* i = dynamic_cast<SaveGameDetour*>(MemoryManager::i());

	const auto len = i->path.size();
	strncpy(retPtr, i->path.c_str(), len + 1);

	return true;
}

std::string SaveGameDetour::GetSaveDataPath() const
{
	INI_Reader ini;
	ini.open("dacom.ini", false);

	ini.find_header("DACOM");

	std::string saveDataPath = ExpandEnvironmentVariables(std::string(R"(%UserProfile%\My Documents\My Games\Freelancer)"));

	while (ini.read_value())
	{
		if (ini.is_value("SavePath"))
		{
			saveDataPath = ExpandEnvironmentVariables(std::string(ini.get_value_string()));
		}
	}

	const std::filesystem::path trimmed = Trim(saveDataPath);

	try
	{
		if (trimmed.empty())
		{
			throw std::runtime_error("SavePath was not defined, but INI key existed within dacom.ini");
		}

		if (!std::filesystem::exists(trimmed))
		{
			std::filesystem::create_directories(trimmed);
		}
	}
	catch (const std::exception& ex)
	{
		MessageBox(GetFLServerHwnd(), ex.what(), "Unable to handle SavePath", MB_ICONERROR);
	}

	return trimmed.string();
}

void SaveGameDetour::InitHook()
{
	auto* i = MemoryManager::i();

	byte* bytes = i->Allocate(5);
	auto origFunc = (GetUserDataPathSig)(GetProcAddress(i->common, "?GetUserDataPath@@YA_NQAD@Z"));

	std::pair<byte*, GetUserDataPathSig> pair {bytes, origFunc};

	dynamic_cast<SaveGameDetour*>(i)->getUserDataPath = pair;

	Detour(PBYTE(origFunc), GetUserDataPathDetour, bytes);

	path = std::format("{}\0", GetSaveDataPath());

	char arr[255];
	GetUserDataPath(arr);
	printf("hi");
}

void SaveGameDetour::DestroyHook()
{
	auto* i = MemoryManager::i();
	UnDetour(PBYTE(i->getUserDataPath.second), i->getUserDataPath.first);

	i->DeAllocate(i->getUserDataPath.first, 5);
	i->getUserDataPath = {};
}
