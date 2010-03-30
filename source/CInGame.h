#include "CCmds.h"

#ifndef _CINGAME_
#define _CINGAME_

class CInGame : public CCmds
{
public:
	uint iClientID;
	wstring wscAdminName;
	void DoPrint(wstring wscText);
	void ReadRights(string scIniFile);
	wstring GetAdminName();
};

#endif
