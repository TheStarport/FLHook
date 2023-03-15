#include "PCH.hpp"
#include "MemoryManager.hpp"

void MemoryManager::AddHooks()
{
	InitHook();
}

void MemoryManager::RemoveHooks()
{
	DestroyHook();
}

