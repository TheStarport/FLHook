#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int __stdcall DisconnectPacketSent(uint iClientID)
{
	TRY_HOOK
	{
		uint iShip = 0;
		pub::Player::GetShip(iClientID, iShip);
		if (set_iDisconnectDelay && iShip)
		{ // in space
			ClientInfo[iClientID].tmF1TimeDisconnect = timeInMS() + set_iDisconnectDelay;
			return 0; // don't pass on
		}
	}
	CATCH_HOOK({})

	return 1; // pass on
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

FARPROC g_OldDisconnectPacketSent;

__declspec(naked) void Naked__DisconnectPacketSent()
{
	__asm {
        pushad 
        mov eax, [esi+0x68]
        push eax
        call DisconnectPacketSent
        cmp eax, 0
        jz suppress
        popad
        jmp [g_OldDisconnectPacketSent]
suppress:
        popad
        mov eax, [hModDaLib]
        add eax, ADDR_DALIB_DISC_SUPPRESS
        jmp eax
	}
}
