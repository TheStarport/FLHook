#include "PCH.hpp"

#include "API/FLServer/Client.hpp"
#include "Global.hpp"

/**************************************************************************************************************
// misc flserver engine function hooks
**************************************************************************************************************/

namespace IEngineHook
{
    /**************************************************************************************************************
    // ship create & destroy
    **************************************************************************************************************/

    FARPROC g_OldInitCShip;

    void __stdcall CShip__Init(CShip* ship) { CallPlugins(&Plugin::OnCShipInit, ship); }

    __declspec(naked) void Naked__CShip__Init()
    {
        __asm {
			push ecx
			push [esp+8]
			call g_OldInitCShip
			call CShip__Init
			ret 4
        }
    }

    FARPROC g_OldDestroyCShip;

    void __stdcall CShip__Destroy(CShip* ship) { CallPlugins(&Plugin::OnCShipDestroy, ship); }

    __declspec(naked) void Naked__CShip__Destroy()
    {
        __asm {
			push ecx
			push ecx
			call CShip__Destroy
			pop ecx
			jmp g_OldDestroyCShip
        }
    }

    /**************************************************************************************************************
    // flserver memory leak bugfix
    **************************************************************************************************************/

    int __cdecl FreeReputationVibe(const int& p1)
    {
        __asm {
			mov eax, p1
			push eax
			mov eax, [server]
			add eax, 0x65C20
			call eax
			add esp, 4
        }

        return Reputation::Vibe::Free(p1);
    }

    /**************************************************************************************************************
    **************************************************************************************************************/

    void __cdecl UpdateTime(double interval) { Timing::UpdateGlobalTime(interval); }

    /**************************************************************************************************************
    **************************************************************************************************************/

    uint g_LastTicks = 0;

    static void* dummy;

    void __stdcall ElapseTime(float interval)
    {
        dummy = &Server;
        Server.ElapseTime(interval);

        // low server load missile jitter bug fix
        const uint curLoad = GetTickCount() - g_LastTicks;
        if (curLoad < 5)
        {
            const uint fakeLoad = 5 - curLoad;
            Sleep(fakeLoad);
        }
        g_LastTicks = GetTickCount();
    }

    /**************************************************************************************************************
    **************************************************************************************************************/

    int __cdecl DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response)
    {
        //	dockPortIndex == -1, response -> 2 --> Dock Denied!
        //	dockPortIndex == -1, response -> 3 --> Dock in Use
        //	dockPortIndex != -1, response -> 4 --> Dock ok, proceed (dockPortIndex starts from 0 for docking point 1)
        //	dockPortIndex == -1, response -> 5 --> now DOCK!

        auto [override, skip] = CallPlugins<std::optional<DOCK_HOST_RESPONSE>>(&Plugin::OnDockCall, shipId, spaceId, dockPortIndex, response);

        if (skip)
        {
            return 0;
        }

        if (override.has_value())
        {
            response = override.value();
        }

        int retVal = 0;
        TRY_HOOK
        {
            // Print out a message when a player ship docks.
            if (FLHookConfig::c()->chatConfig.dockingMessages && response == DOCK_HOST_RESPONSE::Dock)
            {
                if (const auto client = Hk::Client::GetClientIdByShip(shipId).Raw(); client.has_value())
                {
                    std::wstring msg = L"Traffic control alert: %player has docked.";
                    msg = StringUtils::ReplaceStr(msg, L"%player", (const wchar_t*)Players.GetActiveCharacterName(client.value()));
                    PrintLocalUserCmdText(client.value(), msg, 15000);
                }
            }
            // Actually dock
            retVal = pub::SpaceObj::Dock(shipId, spaceId, dockPortIndex, response);
        }
        CATCH_HOOK({})

        CallPlugins(&Plugin::OnDockCallAfter, shipId, spaceId, dockPortIndex, response);

        return retVal;
    }

    /**************************************************************************************************************
    **************************************************************************************************************/

    FARPROC g_OldLaunchPosition;

    bool __stdcall LaunchPosition(const uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, int dock)
    {
        const LaunchData data = { &obj, position, orientation, dock };
        if (auto [retVal, skip] = CallPlugins<std::optional<LaunchData>>(&Plugin::OnLaunchPosition, spaceId, data); skip && retVal.has_value())
        {
            const auto& val = retVal.value();
            position = val.pos;
            orientation = val.orientation;
            return true;
        }

        return obj.launch_pos(position, orientation, dock);
    }

    __declspec(naked) void Naked__LaunchPosition()
    {
        __asm {
			push ecx // 4
			push [esp+8+8] // 8
			push [esp+12+4] // 12
			push [esp+16+0] // 16
			push ecx
			push [ecx+176]
			call LaunchPosition
			pop ecx
			ret 0x0C
        }
    }

    /**************************************************************************************************************
    **************************************************************************************************************/

    struct LOAD_REP_DATA
    {
            uint repId;
            float attitude;
    };

    struct REP_DATA_LIST
    {
            uint dunno;
            LOAD_REP_DATA* begin;
            LOAD_REP_DATA* end;
    };

    bool __stdcall LoadReputationFromCharacterFile(REP_DATA_LIST* savedReps, LOAD_REP_DATA* repToSave)
    {
        // check of the rep id is valid
        if (repToSave->repId == 0xFFFFFFFF)
        {
            return false; // rep id not valid!
        }

        LOAD_REP_DATA* repIt = savedReps->begin;

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

    FARPROC g_OldLoadReputationFromCharacterFile;

    __declspec(naked) void Naked__LoadReputationFromCharacterFile()
    {
        __asm {
			push ecx // save ecx because thiscall
			push [esp+4+4+8] // rep data
			push ecx // rep data list
			call LoadReputationFromCharacterFile
			pop ecx // recover ecx
			test al, al
			jz abort_lbl
			jmp [g_OldLoadReputationFromCharacterFile]
			abort_lbl:
			ret 0x0C
        }
    }

    /**************************************************************************************************************
    **************************************************************************************************************/
} // namespace IEngineHook
