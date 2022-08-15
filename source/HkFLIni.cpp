#include "Global.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

cpp::result<std::wstring, HkError> HkFLIniGet(const std::variant<uint, std::wstring>& player, const std::wstring& wscKey)
{
	std::wstring ret;
	std::wstring wscDir;
	if (!HKHKSUCCESS(HkGetAccountDirName(player, wscDir)))
		return cpp::fail(CharDoesNotExist);

	std::wstring wscFile;
	HkGetCharFileName(player, wscFile);

	if (const std::string scCharFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl"; HkIsEncoded(scCharFile))
	{
		const std::string scCharFileNew = scCharFile + ".ini";
		if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
			return CouldNotDecodeCharFile;

		ret = stows(IniGetS(scCharFileNew, "Player", wstos(wscKey), ""));
		DeleteFile(scCharFileNew.c_str());
	}
	else
	{
		ret = stows(IniGetS(scCharFile, "Player", wstos(wscKey), ""));
	}

	return ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkFLIniWrite(const std::variant<uint, std::wstring>& player, const std::wstring& wscKey, const std::wstring& wscValue)
{
	std::wstring wscDir;
	if (!HKHKSUCCESS(HkGetAccountDirName(player, wscDir)))
		return CharDoesNotExist;

	std::wstring wscFile;
	HkGetCharFileName(player, wscFile);

	std::string scCharFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
	if (HkIsEncoded(scCharFile))
	{
		std::string scCharFileNew = scCharFile + ".ini";
		if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
			return CouldNotDecodeCharFile;

		IniWrite(scCharFileNew, "Player", wstos(wscKey), wstos(wscValue));

		// keep decoded
		DeleteFile(scCharFile.c_str());
		MoveFile(scCharFileNew.c_str(), scCharFile.c_str());
		/*		if(!flc_encode(scCharFileNew.c_str(), scCharFile.c_str()))
		                        return CouldNotEncodeCharFile;

		                DeleteFile(scCharFileNew.c_str()); */
	}
	else
	{
		IniWrite(scCharFile, "Player", wstos(wscKey), wstos(wscValue));
	}

	return HKE_OK;
}
