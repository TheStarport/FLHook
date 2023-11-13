
#include "PCH.hpp"

#include "Core/IEngineHook.hpp"
#include "Defs/FLHookConfig.hpp"

int __stdcall IEngineHook::DisconnectPacketSent(ClientId client)
{
    TryHook
    {
        uint ship = 0;
        pub::Player::GetShip(client, ship);
        if (FLHookConfig::i()->general.disconnectDelay && ship)
        {
            // in space
            ClientInfo::At(client).tmF1TimeDisconnect = TimeUtils::UnixMilliseconds() + FLHookConfig::i()->general.disconnectDelay;
            return 0; // don't pass on
        }
    }
    CatchHook({})

        return 1; // pass on
}

__declspec(naked) void IEngineHook::NakedDisconnectPacketSent()
{
    __asm {
		pushad
		mov eax, [esi+0x68]
		push eax
		call DisconnectPacketSent
		cmp eax, 0
		jz suppress
		popad
		jmp [IEngineHook::oldDisconnectPacketSent]

		suppress:
		popad
		mov eax, [hModDaLib]
		add eax, ADDR_DALIB_DISC_SUPPRESS
		jmp eax
    }
}
