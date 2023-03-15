#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <array>

template<typename CallSig>
class FunctionDetour final
{
	CallSig originalFunc;
	PBYTE data;
	std::allocator<unsigned char> alloc;

	FunctionDetour(const FunctionDetour&) = delete;
	FunctionDetour& operator=(FunctionDetour) = delete;

  public:
	CallSig GetOriginalFunc() { return originalFunc; }

	explicit FunctionDetour(CallSig origFunc) : originalFunc(origFunc) { data = alloc.allocate(5); }
	~FunctionDetour() { alloc.deallocate(data, 5); }

	void Detour(const CallSig hookedFunc)
	{
		DWORD oldProtection = 0;  // Create a DWORD for VirtualProtect calls to allow us to write.
		std::array<byte, 5> patch; // We need to change 5 bytes and I'm going to use memcpy so this is the simplest way.
		patch[0] = 0xE9;           // Set the first byte of the byte array to the op code for the JMP instruction.
		VirtualProtect((void*)originalFunc, 5, PAGE_EXECUTE_READWRITE, &oldProtection); // Allow us to write to the memory we need to change
		DWORD relativeAddress = (DWORD)hookedFunc - (DWORD)originalFunc - 5;            // Calculate the relative JMP address.
		memcpy(&patch[1], &relativeAddress, 4);                                        // Copy the relative address to the byte array.
		memcpy(data, originalFunc, 5);
		memcpy(originalFunc, patch.data(), 5);                           // Change the first 5 bytes to the JMP instruction.
		VirtualProtect((void*)originalFunc, 5, oldProtection, nullptr); // Set the protection back to what it was.
	}

	void UnDetour()
	{
		DWORD oldProtection = 0;                                                        // Create a DWORD for VirtualProtect calls to allow us to write.
		VirtualProtect((void*)originalFunc, 5, PAGE_EXECUTE_READWRITE, &oldProtection); // Allow us to write to the memory we need to change
		memcpy(originalFunc, data, 5);
		VirtualProtect((void*)originalFunc, 5, oldProtection, nullptr); // Set the protection back to what it was.
	}
};