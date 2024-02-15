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
        if (FLHook::GetConfig().general.disconnectDelay && ship)
        {
            // in space
            client.GetData().timeDisconnect = TimeUtils::UnixTime<std::chrono::milliseconds>() + FLHook::GetConfig().general.disconnectDelay;
            return 0; // don't pass on
        }
    }
    CatchHook({})

        return 1; // pass on
}

IEngineHook::DisconnectPacketSentAssembly::DisconnectPacketSentAssembly()
{
    pushad();
    mov(eax, dword[esi+0x68]);
    push(eax);
    call(DisconnectPacketSent);
    cmp(eax, 0);
    popad();
    jz(".suppress");
    jmp(dword[IEngineHook::oldDisconnectPacketSent]);

    L(".suppress");
    mov(eax, dword[FLHook::dalibDll]);
    add(eax, static_cast<DWORD>(AddressList::DalibDiscSuppress));
    jmp(eax);
}
