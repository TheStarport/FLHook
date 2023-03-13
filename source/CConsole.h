#ifndef _CCONSOLE_
#define _CCONSOLE_

#include <FLHook.hpp>

class CConsole : public CCmds
{
public:
	CConsole() { this->rights = RIGHT_SUPERADMIN; };
	EXPORT void DoPrint(const std::string& text) override;
	std::wstring GetAdminName() override;
};

#endif
