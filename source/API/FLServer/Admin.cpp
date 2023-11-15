#include "PCH.hpp"

#include "Core/ApiInternal.hpp"
#include "Core/FLHook.hpp"

Action<void, Error> ChangeNPCSpawn(bool disable)
{
    if (CoreGlobals::c()->disableNpcs && disable)
    {
        return { {} };
    }
    if (!CoreGlobals::c()->disableNpcs && !disable)
    {
        return { {} };
    }

    char jump[1];
    char cmp[1];
    if (disable)
    {
        jump[0] = '\xEB';
        cmp[0] = '\xFF';
    }
    else
    {
        jump[0] = '\x75';
        cmp[0] = '\xF9';
    }

    auto address = FLHook::Offset(FLHook::BinaryType::Content, AddressList::DisableNpcSpawns1);
    MemUtils::WriteProcMem(address, &jump, 1);
    address = FLHook::Offset(FLHook::BinaryType::Content, AddressList::DisableNpcSpawns2);
    MemUtils::WriteProcMem(address, &cmp, 1);
    g_bNPCDisabled = disable;
    return { {} };
}

Fuse* GetFuseFromID(uint fuseId)
{
    int dunno = 0;
    Fuse* fuse = nullptr;
    __asm {
			mov edx, 0x6CFD390
			call edx

			lea ecx, fuseId
			push ecx
			lea ecx, dunno
			push ecx
			mov ecx, eax
			mov edx, 0x6D15D10
			call edx
			mov edx, [dunno]
			mov edi, [edx+0x10]
			mov fuse, edi
    }
    return fuse;
}

/// Return the CEqObj from the IObjRW
__declspec(naked) CEqObj* __stdcall GetEqObjFromObjRW_(IObjRW* objRW)
{
    __asm {
			push ecx
			push edx
			mov ecx, [esp+12]
			mov edx, [ecx]
			call dword ptr[edx+0x150]
			pop edx
			pop ecx
			ret 4
    }
}

CEqObj* GetEqObjFromObjRW(IObjRW* objRW) { return GetEqObjFromObjRW_(objRW); }
