#include "MemoryManager.hpp"

void MemoryManager::AddHooks()
{
	SaveGameDetour::InitHook();
}

void MemoryManager::RemoveHooks()
{
	SaveGameDetour::DestroyHook();
}