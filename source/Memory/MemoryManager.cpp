#include "MemoryManager.hpp"

void MemoryManager::AddHooks()
{
	SaveGameDetour::InitHook();
}

void MemoryManager::RemoveHooks()
{
	SaveGameDetour::DestroyHook();
}

byte* MemoryManager::Allocate(int size)
{
	return allocator.allocate(size);
}

void MemoryManager::DeAllocate(byte* ptr, int size)
{
	allocator.deallocate(ptr, size);
}
