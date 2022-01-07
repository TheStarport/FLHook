#pragma once

#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode;

void ClearClientMark(uint iClientID);
void HkUnMarkAllObjects(uint iClientID);
char HkUnMarkObject(uint iClientID, uint iObject);
char HkMarkObject(uint iClientID, uint iObject);

struct MARK_INFO {
    bool bMarkEverything;
    bool bIgnoreGroupMark;
    float fAutoMarkRadius;
    std::vector<uint> vMarkedObjs;
    std::vector<uint> vDelayedSystemMarkedObjs;
    std::vector<uint> vAutoMarkedObjs;
    std::vector<uint> vDelayedAutoMarkedObjs;
};

struct DELAY_MARK {
    uint iObj;
    mstime time;
};
std::string ftos(float f);
