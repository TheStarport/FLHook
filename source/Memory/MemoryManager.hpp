#pragma once

#include <Global.hpp>

class SaveGameDetour
{
	std::string path;

	static bool GetUserDataPathDetour(char* retPtr);
	std::string GetSaveDataPath() const;

protected:
	void InitHook();
	void DestroyHook();
};

class MemoryManager final : public SaveGameDetour, public Singleton<MemoryManager>
{
public:
	MemoryManager() = default;
	void AddHooks();
	void RemoveHooks();
};
