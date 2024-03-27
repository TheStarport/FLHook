#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "Core/IEngineHook.hpp"
#include "Defs/FLHookConfig.hpp"

bool __stdcall IEngineHook::DisconnectPacketSent(ClientId client)
{
    Logger::Log(LogLevel::Info, L"Client disconnecting!!");
    TryHook
    {
        uint ship = 0;
        pub::Player::GetShip(client.GetValue(), ship);
        if (FLHook::GetConfig().general.disconnectDelay && ship)
        {
            // in space
            client.GetData().timeDisconnect = TimeUtils::UnixTime<std::chrono::milliseconds>() + FLHook::GetConfig().general.disconnectDelay;
            return false; // suppress
        }
    }
    CatchHook({});

    return true; // don't suppress
}

IEngineHook::DisconnectPacketSentAssembly::DisconnectPacketSentAssembly()
{
    pushad();
    mov(eax, dword[esi+0x68]);
    push(eax);
    call((void*)DisconnectPacketSent);
    test(al, al);
    popad();
    jz(".suppress");
    jmp(ptr[&oldDisconnectPacketSent]);

    L(".suppress");
    mov(eax, ptr[FLHook::dalibDll]);
    add(eax, static_cast<DWORD>(AddressList::DalibDiscSuppress));
    jmp(eax);
}
