#include <PCH.hpp>
#include "Tools/CommandLineParser.hpp"
#include <shellapi.h>



CommandLineParser::CommandLineParser()
{
	int argc = 0;
	const LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for (int i = 1; i < argc; ++i)
	{
		tokens.emplace_back(wstos(argv[i]));
	}
}

// Get the value following a command line argument
// Example, say this was your command line. something.exe -w -de -val=something
// GetCmdOption on -w or -de would return an empty string, while doing it on -val would return "something".
std::string CommandLineParser::GetCmdOption(const std::string& option) const
{
	if (const auto itr = std::ranges::find_if(this->tokens, [option](const std::string& str) { return str.find(option) != std::string::npos; });
		itr != this->tokens.end() && itr->find('=') != std::string::npos)
	{
		return itr->substr(itr->find('=') + 1);
	}

	static const std::string Empty;
	return Empty;
}

bool CommandLineParser::CmdOptionExists(const std::string& option) const
{
	return std::ranges::find(this->tokens, option) != this->tokens.end();
}
