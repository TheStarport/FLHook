#include "PCH.hpp"

#include "Core/IEngineHook.hpp"

void __stdcall IEngineHook::CShipInit(CShip* ship) { CallPlugins(&Plugin::OnCShipInit, ship); }

IEngineHook::CShipInitAssembly::CShipInitAssembly()
{
    push(ecx);
    push(dword[esp + 8]);
    call(oldInitCShip);
    call(CShipInit);
    ret(4);
}

IEngineHook::CallAndRet::CallAndRet(void* toCall, void* ret)
{
    push(ecx); // Reserve ecx
    push(ecx);
    call(toCall);
    pop(ecx); // Restore ecx
    jmp(ret);
}

void __stdcall IEngineHook::CShipDestroy(CShip* ship) { CallPlugins(&Plugin::OnCShipDestroy, ship); }

void __stdcall IEngineHook::CLootDestroy(CLoot* loot) { CallPlugins(&Plugin::OnCLootDestroy, loot); }

void __stdcall IEngineHook::CSolarDestroy(CSolar* solar) { CallPlugins(&Plugin::OnCSolarDestroy, solar); }

int IEngineHook::FreeReputationVibe(const int& p1)
{
    struct Code : Xbyak::CodeGenerator
    {
            explicit Code(int p)
            {
                mov(eax, p);
                push(eax);
                mov(eax, dword[FLHook::serverDll]);
                add(eax, 0x65C20);
                call(eax);
                add(esp, 4);
            }
    };

    Code code(p1);

    static auto call = code.getCode<void (*)()>();
    call();

    return Reputation::Vibe::Free(p1);
}

void IEngineHook::UpdateTime(const double interval) { Timing::UpdateGlobalTime(interval); }

static void* dummy;

void __stdcall IEngineHook::ElapseTime(float interval)
{
    // TODO: Figure out if this is even needed
    dummy = &Server;
    Server.ElapseTime(interval);

    // low server load missile jitter bug fix
    const uint curLoad = GetTickCount() - lastTicks;
    if (curLoad < 5)
    {
        const uint fakeLoad = 5 - curLoad;
        Sleep(fakeLoad);
    }
    lastTicks = GetTickCount();
}

int IEngineHook::DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response)
{
    //	dockPortIndex == -1, response -> 2 --> Dock Denied!
    //	dockPortIndex == -1, response -> 3 --> Dock in Use
    //	dockPortIndex != -1, response -> 4 --> Dock ok, proceed (dockPortIndex starts from 0 for docking point 1)
    //	dockPortIndex == -1, response -> 5 --> now DOCK!

    auto [override, skip] = CallPlugins<std::optional<DOCK_HOST_RESPONSE>>(&Plugin::OnDockCall, ShipId(shipId), ObjectId(spaceId), dockPortIndex, response);

    if (skip)
    {
        return 0;
    }

    if (override.has_value())
    {
        response = override.value();
    }

    int retVal = 0;
    TryHook
    {
        // Print out a message when a player ship docks.
        if (FLHook::GetConfig().chatConfig.dockingMessages && response == DOCK_HOST_RESPONSE::Dock)
        {
            if (const auto client = ShipId(shipId).GetPlayer().value_or(ClientId()))
            {
                std::wstring msg = L"Traffic control alert: %player has docked.";
                msg = StringUtils::ReplaceStr(msg, std::wstring_view(L"%player"), client.GetCharacterName().Unwrap());
                client.MessageLocal(msg, 15000.0f);
            }
        }
        // Actually dock
        retVal = pub::SpaceObj::Dock(shipId, spaceId, dockPortIndex, response);
    }
    CatchHook({})

        CallPlugins(&Plugin::OnDockCallAfter, ShipId(shipId), ObjectId(spaceId), dockPortIndex, response);

    return retVal;
}

bool __stdcall IEngineHook::LaunchPosition(const uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, int dock)
{
    const LaunchData data = { &obj, position, orientation, dock };
    if (auto [retVal, skip] = CallPlugins<std::optional<LaunchData>>(&Plugin::OnLaunchPosition, ObjectId(spaceId), data); skip && retVal.has_value())
    {
        const auto& val = retVal.value();
        position = val.pos;
        orientation = val.orientation;
        return true;
    }

    return obj.launch_pos(position, orientation, dock);
}

IEngineHook::LaunchPositionAssembly::LaunchPositionAssembly()
{
    push(ecx);
    push(dword[esp+16]);
    push(dword[esp+16]);
    push(dword[esp+16]);
    push(ecx);
    push(dword[ecx+176]);
    call(LaunchPosition);
    pop(ecx);
    ret(0xC);
}

auto __stdcall IEngineHook::LoadReputationFromCharacterFile(const RepDataList* savedReps, const LoadRepData* repToSave) -> bool
{
    // check of the rep id is valid
    if (repToSave->repId == 0xFFFFFFFF)
    {
        return false; // rep id not valid!
    }

    const LoadRepData* repIt = savedReps->begin;

    while (repIt != savedReps->end)
    {
        if (repIt->repId == repToSave->repId)
        {
            return false; // we already saved this rep!
        }

        repIt++;
    }

    // everything seems fine, add
    return true;
}

IEngineHook::LoadReputationFromCharacterFileAssembly::LoadReputationFromCharacterFileAssembly()
{
    push(ecx);
    push(dword[esp+16]);
    push(ecx);
    call(LoadReputationFromCharacterFile);
    pop(ecx);
    test(al, al);
    jz(".abort");
    jmp(dword[oldLoadReputationFromCharacterFile]);

    inLocalLabel();
    L(".abort");
    ret(0xC);
}