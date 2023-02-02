#include "Global.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int __stdcall DisconnectPacketSent(ClientId client)
{
	TRY_HOOK
	{
		uint ship = 0;
		pub::Player::GetShip(client, ship);
		if (FLHookConfig::i()->general.disconnectDelay && ship)
		{ // in space
			ClientInfo[client].tmF1TimeDisconnect = timeInMS() + FLHookConfig::i()->general.disconnectDelay;
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
