// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
#pragma once
#include <Hook.h>

void HkIClientImpl__Startup__Inner(uint, uint) {
    // load the universe directly before the server becomes internet accessible
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

// Doesn't call a Client method, so we need a custom hook
bool HkIClientImpl::DispatchMsgs() {
    cdpServer->DispatchMsgs(); // calls IServerImpl functions, which also call
                               // HkIClientImpl functions
    return true;
}