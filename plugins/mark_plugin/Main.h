#pragma once

#include <FLHook.h>
#include <plugin.h>

static int set_iPluginDebug = 0;
ReturnCode returncode;

typedef void (*_UserCmdProc)(uint, const std::wstring &);

struct USERCMD {
    wchar_t *wszCmd;
    _UserCmdProc proc;
};

#define IS_CMD(a) !wscCmd.compare(L##a)

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
