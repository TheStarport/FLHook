#include "Hook.h"

std::list<stHelpEntry> lstHelpEntries;
bool get_bTrue(uint iClientID)
{
	return true;
}
void HkAddHelpEntry(
    const std::wstring& wscCommand, const std::wstring& wscArguments, const std::wstring& wscShortHelp,
    const std::wstring& wscLongHelp, _HelpEntryDisplayed fnIsDisplayed)
{
	for (auto& he : lstHelpEntries)
	{
		if (he.wszCommand == wscCommand && he.wszArguments == wscArguments)
			return;
	}
	stHelpEntry he;
	he.fnIsDisplayed = fnIsDisplayed;
	he.wszArguments = wscArguments;
	he.wszCommand = wscCommand;
	he.wszLongHelp = wscLongHelp;
	for (uint a = 0; a < he.wszLongHelp.length(); a++)
	{
		if (he.wszLongHelp[a] == '\t')
		{
			he.wszLongHelp = he.wszLongHelp.replace(a, 1, L"  ");
			a += 3;
		}
	}

	he.wszShortHelp = wscShortHelp;
	lstHelpEntries.push_back(he);
}
void HkRemoveHelpEntry(const std::wstring& wscCommand, const std::wstring& wscArguments)
{
	for (auto he = lstHelpEntries.begin(); he != lstHelpEntries.end(); ++he)
	{
		if (he->wszCommand == wscCommand && he->wszArguments == wscArguments)
			lstHelpEntries.erase(he);
	}
}
