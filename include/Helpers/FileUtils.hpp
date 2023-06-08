#pragma once

class FileUtils
{
	static cpp::result<std::wstring, Error> FileToBuffer(const std::wstring& fileLocation);
	static void BufferToFile(const std::wstring& fileLocation, std::wstring_view newFileData);

  public:
	FileUtils() = delete;

	static cpp::result<std::wstring, Error> ReadCharacterFile(std::wstring_view characterName);
	static Action<void> WriteCharacterFile(std::wstring_view characterName, std::wstring_view newFileData);
	static std::wstring SaveDataPath();
};