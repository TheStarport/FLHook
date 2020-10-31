#include "Hook.h"

void HkIClientImpl__Startup__Inner(uint iDunno, uint iDunno2) {
    // load universe / we load the universe directly before the server becomes
    // internet accessible
    lstBases.clear();
    Universe::IBase *base = Universe::GetFirstBase();
    while (base) {
        BASE_INFO bi;
        bi.bDestroyed = false;
        bi.iObjectID = base->lSpaceObjID;
        char *szBaseName = "";
        __asm {
            pushad
            mov ecx, [base]
            mov eax, [base]
            mov eax, [eax]
            call [eax+4]
            mov [szBaseName], eax
            popad
        }

        bi.scBasename = szBaseName;
        bi.iBaseID = CreateID(szBaseName);
        lstBases.push_back(bi);
        pub::System::LoadSystem(base->iSystemID);

        base = Universe::GetNextBase();
    }
}

bool HkIClientImpl::DispatchMsgs() {

    /*	long lRet;
            char *tmp;
            WriteProcMem(&tmp, &Client, 4);
            WriteProcMem(&Client, &OldClient, 4);
            HookClient->DispatchMsgs();
            __asm { mov [lRet], eax }
            WriteProcMem(&Client, &tmp, 4); */

    cdpserver->DispatchMsgs(); // calls IServerImpl functions, which also call
                               // HkIClientImpl functions
    return true;
}