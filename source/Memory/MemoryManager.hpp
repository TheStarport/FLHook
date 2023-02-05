#pragma once

#include <Global.hpp>

class SaveGameDetour
{
	using GetUserDataPathSig = bool (*)(char*);
	std::pair<byte*, GetUserDataPathSig> getUserDataPath;
	std::string path;

	static bool GetUserDataPathDetour(char* retPtr);
	std::string GetSaveDataPath() const;

  public:
	virtual void InitHook();
	virtual void DestroyHook();
};

class MemoryManager final : public SaveGameDetour, public Singleton<MemoryManager>
{
	std::allocator<byte> allocator;

  public:
	MemoryManager() { common = GetModuleHandle("common.dll"); }

	HMODULE common;

	void AddHooks();
	void RemoveHooks();

	byte* Allocate(int size);
	void DeAllocate(byte* ptr, int size);
};