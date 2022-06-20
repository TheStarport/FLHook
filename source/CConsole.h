#ifndef _CCONSOLE_
#define _CCONSOLE_

#include <FLHook.hpp>

class CConsole : public CCmds
{
  public:
	CConsole() { this->rights = RIGHT_SUPERADMIN; };
	EXPORT void DoPrint(const std::wstring& wscText);
	std::wstring GetAdminName();
};

#endif
