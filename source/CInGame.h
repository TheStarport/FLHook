#include "CCmds.h"

#ifndef _CINGAME_
#define _CINGAME_

class CInGame : public CCmds
{
public:
	uint iClientID;
	std::wstring wscAdminName;
	void DoPrint(const std::wstring &wscText);
	void ReadRights(const std::string &scIniFile);
	std::wstring GetAdminName();
};

#endif
