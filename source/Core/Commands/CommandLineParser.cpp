#include <PCH.hpp>

#include <shellapi.h>
#include "Core/Commands/CommandLineParser.hpp"



CommandLineParser::CommandLineParser()
{
	int argc = 0;
	const LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for (int i = 1; i < argc; ++i)
	{
		tokens.emplace_back(argv[i]);
	}
}

// Get the value following a command line argument
// Example, say this was your command line. something.exe -w -de -val=something
// GetCmdOption on -w or -de would return an empty string, while doing it on -val would return "something".
std::wstring CommandLineParser::GetCmdOption(const std::wstring& option) const
{
	if (const auto itr = std::ranges::find_if(this->tokens, [option](const std::wstring& str) { return str.find(option) != std::wstring::npos; });
		itr != this->tokens.end() && itr->find('=') != std::wstring::npos)
	{
		return itr->substr(itr->find('=') + 1);
	}

	static const std::wstring Empty;
	return Empty;
}

bool CommandLineParser::CmdOptionExists(const std::wstring& option) const
{
	return std::ranges::find(this->tokens, option) != this->tokens.end();
}
