#include "PCH.hpp"

HWND MemUtils::GetFlServerHwnd()
{
	auto* unkThis = reinterpret_cast<void*>(0x00426C58);
	return *reinterpret_cast<HWND*>(*(static_cast<PDWORD>(unkThis) + 8) + 32);
}

void MemUtils::SwapBytes(void* ptr, uint len)
{
	if (len % 4)
		return;

	for (uint i = 0; i < len; i += 4)
	{
		char* ptr1 = static_cast<char*>(ptr) + i;
		unsigned long temp;
		memcpy(&temp, ptr1, 4);
		const auto ptr2 = (char*)&temp;
		memcpy(ptr1, ptr2 + 3, 1);
		memcpy(ptr1 + 1, ptr2 + 2, 1);
		memcpy(ptr1 + 2, ptr2 + 1, 1);
		memcpy(ptr1 + 3, ptr2, 1);
	}
}

void MemUtils::WriteProcMem(void* address, const void* mem, uint size)
{
	const HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD old;
	VirtualProtectEx(hProc, address, size, PAGE_EXECUTE_READWRITE, &old);
	WriteProcessMemory(hProc, address, mem, size, nullptr);
	CloseHandle(hProc);
}

void MemUtils::MemUtils::ReadProcMem(void* address, void* mem, uint size)
{
	const HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD old;
	VirtualProtectEx(hProc, address, size, PAGE_EXECUTE_READWRITE, &old);
	ReadProcessMemory(hProc, address, mem, size, nullptr);
	CloseHandle(hProc);
}

FARPROC MemUtils::MemUtils::PatchCallAddr(char* mod, DWORD installAddress, const char* hookFunction)
{
	DWORD relAddr;
	MemUtils::MemUtils::ReadProcMem(mod + installAddress + 1, &relAddr, 4);

	const DWORD offset = (DWORD)hookFunction - (DWORD)(mod + installAddress + 5);
	MemUtils::WriteProcMem(mod + installAddress + 1, &offset, 4);

	return (FARPROC)(mod + relAddr + installAddress + 5);
}
