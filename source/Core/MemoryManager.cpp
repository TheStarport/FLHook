#include "PCH.hpp"

#include "Core/MemoryManager.hpp"

void MemoryManager::AddHooks() { InitHook(); }
void MemoryManager::RemoveHooks() { DestroyHook(); }