#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "Core/IEngineHook.hpp"
#include "Defs/FLHookConfig.hpp"

int __stdcall IEngineHook::DisconnectPacketSent(ClientId client)
{
    TryHook
    {
        uint ship = 0;
        pub::Player::GetShip(client.GetValue(), ship);
        if (FLHookConfig::i()->general.disconnectDelay && ship)
        {
            // in space
            client.GetData().timeDisconnect = TimeUtils::UnixTime<std::chrono::milliseconds>() + FLHookConfig::i()->general.disconnectDelay;
            return 0; // don't pass on
        }
    }
    CatchHook({})

        return 1; // pass on
}

static constexpr DWORD dalibAddr = static_cast<DWORD>(AddressList::DalibDiscSuppress); // NOLINT
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
		mov eax, [FLHook::dalibDll]
		add eax, dalibAddr
		jmp eax
    }
}
