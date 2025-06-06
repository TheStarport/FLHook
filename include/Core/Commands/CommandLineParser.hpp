#pragma once

class DLL CommandLineParser
{
	std::vector<std::wstring> tokens;

  public:
	CommandLineParser();

	// Get the value following a command line argument
	// Example, say this was your command line. something.exe -w -de -val=something
	// GetCmdOption on -w or -de would return an empty string, while doing it on -val would return "something".
	[[nodiscard]] std::wstring GetCmdOption(const std::wstring& option) const;
	[[nodiscard]] bool CmdOptionExists(const std::wstring& option) const;
};