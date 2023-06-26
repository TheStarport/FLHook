#include "PCH.hpp"

#include "Core/MemoryManager.hpp"
#include <Core/Detour.hpp>

using GetUserDataPathSig = bool (*)(char*);

bool SaveGameDetour::GetUserDataPathDetour(char* retPtr)
{
    const auto* i = dynamic_cast<SaveGameDetour*>(MemoryManager::i());

    const auto len = i->path.size();
    strncpy_s(retPtr, len + 1, StringUtils::wstos(i->path).c_str(), MAX_PATH);

    return true;
}

std::wstring SaveGameDetour::GetSaveDataPath() const
{
    INI_Reader ini;
    ini.open("dacom.ini", false);

    ini.find_header("DACOM");

    std::wstring saveDataPath = StringUtils::ExpandEnvironmentVariables(std::wstring(LR"(%UserProfile%\My Documents\My Games\Freelancer)"));

    while (ini.read_value())
    {
        if (ini.is_value("SavePath"))
        {
            saveDataPath = StringUtils::ExpandEnvironmentVariables(StringUtils::stows(std::string(ini.get_value_string())));
        }
    }

    const std::filesystem::path trimmed = StringUtils::Trim(saveDataPath);

    try
    {
        if (trimmed.empty())
        {
            throw std::runtime_error("SavePath was not defined, but INI key existed within dacom.ini");
        }

        if (!exists(trimmed))
        {
            create_directories(trimmed);
        }
    }
    catch (const std::exception& ex)
    {
        MessageBoxA(MemUtils::GetFlServerHwnd(), ex.what(), "Unable to handle SavePath", MB_ICONERROR);
    }

    return trimmed.wstring();
}

const auto detour =
    std::make_unique<FunctionDetour<GetUserDataPathSig>>(GetUserDataPathSig(GetProcAddress(GetModuleHandle(L"common.dll"), "?GetUserDataPath@@YA_NQAD@Z")));

void SaveGameDetour::InitHook()
{
    detour->Detour(GetUserDataPathDetour);

    path = std::format(L"{}\0", GetSaveDataPath());

    char arr[MAX_PATH];
    GetUserDataPath(arr);
}

void SaveGameDetour::DestroyHook() { detour->UnDetour(); }
