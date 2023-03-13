#ifndef _CCONSOLE_
#define _CCONSOLE_

#include <FLHook.hpp>

class CConsole : public CCmds
{
public:
	CConsole() { this->rights = 0; };
	EXPORT void DoPrint(const std::string& text) override;
	std::wstring GetAdminName() override;
};

#endif
