#include "hook.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"

std::shared_ptr<spdlog::logger> FLHookLog = nullptr;
std::shared_ptr<spdlog::logger> CheaterLog = nullptr;
std::shared_ptr<spdlog::logger> KickLog = nullptr;
std::shared_ptr<spdlog::logger> ConnectsLog = nullptr;
std::shared_ptr<spdlog::logger> AdminCmdsLog = nullptr;
std::shared_ptr<spdlog::logger> SocketCmdsLog = nullptr;
std::shared_ptr<spdlog::logger> UserCmdsLog = nullptr;
std::shared_ptr<spdlog::logger> PerfTimersLog = nullptr;
std::shared_ptr<spdlog::logger> FLHookDebugLog = nullptr;
std::shared_ptr<spdlog::logger> WinDebugLog = nullptr;

bool InitLogs()
{
	try
	{
		FLHookLog = spdlog::basic_logger_mt<spdlog::async_factory>("FLHook", "flhook_logs/FLHook.log");
		CheaterLog =
		    spdlog::basic_logger_mt<spdlog::async_factory>("flhook_cheaters", "flhook_logs/flhook_cheaters.log");
		KickLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_kicks", "flhook_logs/flhook_kicks.log");
		ConnectsLog =
		    spdlog::basic_logger_mt<spdlog::async_factory>("flhook_connects", "flhook_logs/flhook_connects.log");
		AdminCmdsLog =
		    spdlog::basic_logger_mt<spdlog::async_factory>("flhook_admincmds", "flhook_logs/flhook_admincmds.log");
		SocketCmdsLog =
		    spdlog::basic_logger_mt<spdlog::async_factory>("flhook_socketcmds", "flhook_logs/flhook_socketcmds.log");
		UserCmdsLog =
		    spdlog::basic_logger_mt<spdlog::async_factory>("flhook_usercmds", "flhook_logs/flhook_usercmds.log");
		PerfTimersLog =
		    spdlog::basic_logger_mt<spdlog::async_factory>("flhook_perftimers", "flhook_logs/flhook_perftimers.log");

		if (IsDebuggerPresent())
		{
			WinDebugLog = spdlog::create_async<spdlog::sinks::msvc_sink_mt>("windows_debug");
			WinDebugLog->set_level(spdlog::level::debug);
		}

		if (set_bDebug)
		{
			char szDate[64];
			time_t tNow = time(0);
			tm t;
			localtime_s(&t, &tNow);
			strftime(szDate, sizeof szDate, "%d.%m.%Y_%H.%M", &t);

			std::string sDebugLog = "./flhook_logs/debug/FLHookDebug_" + (std::string)szDate;
			sDebugLog += ".log";

			FLHookDebugLog = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", sDebugLog);
			FLHookDebugLog->set_level(spdlog::level::debug);
		}
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		Console::ConErr(L"Log initialization failed: %s", ex.what());
		return false;
	}
	return true;
}

void AddLog(LogType LogType, std::wstring wStr, ...)
{
	va_list marker;
	va_start(marker, wStr);

	wchar_t wszBuf[1024 * 8] = L"";

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wStr.c_str(), marker);
	std::string scText = wstos(wszBuf);

	switch (LogType)
	{
		case Cheater:
			CheaterLog->info(scText);
			CheaterLog->flush();
			break;
		case Kick:
			KickLog->info(scText);
			KickLog->flush();
			break;
		case Connects:
			ConnectsLog->info(scText);
			ConnectsLog->flush();
			break;
		case AdminCmds:
			AdminCmdsLog->info(scText);
			AdminCmdsLog->flush();
			break;
		case UserLogCmds:
			UserCmdsLog->info(scText);
			UserCmdsLog->flush();
			break;
		case SocketCmds:
			SocketCmdsLog->info(scText);
			SocketCmdsLog->flush();
			break;
		case PerfTimers:
			PerfTimersLog->info(scText);
			PerfTimersLog->flush();
			break;
		case Debug:
			if (FLHookDebugLog)
			{
				FLHookDebugLog->debug(scText);
				FLHookDebugLog->flush();
			}
			break;
		case Normal:
			FLHookLog->info(scText);
			break;
		case Error:
			FLHookLog->error(scText);
			break;
	}

	if (IsDebuggerPresent())
	{
		WinDebugLog->debug(scText);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkHandleCheater(uint iClientID, bool bBan, std::wstring wscReason, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, wscReason);

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wscReason.c_str(), marker);

	HkAddCheaterLog(iClientID, wszBuf);

	if (wscReason[0] != '#' && Players.GetActiveCharacterName(iClientID))
	{
		std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);

		wchar_t wszBuf2[500];
		swprintf_s(wszBuf2, L"Possible cheating detected: %s", wscCharname.c_str());
		HkMsgU(wszBuf2);
	}

	if (bBan)
		HkBan(iClientID, true);
	if (wscReason[0] != '#')
		HkKick(iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddCheaterLog(std::variant<uint, std::wstring> player, const std::wstring& wscReason)
{
	const uint iClientID = HkExtractClientId(player);

	CAccount* acc = Players.FindAccountFromClientID(iClientID);
	std::wstring wscAccountDir = L"???";
	std::wstring wscAccountID = L"???";
	if (acc)
	{
		HkGetAccountDirName(acc, wscAccountDir);
		wscAccountID = HkGetAccountID(acc);
	}

	std::wstring wscHostName = L"???";
	std::wstring wscIp = L"???";

	wscHostName = ClientInfo[iClientID].wscHostname;
	HkGetPlayerIP(iClientID, wscIp);

	const auto wscCharacterName = Players.GetActiveCharacterName(iClientID);

	AddLog(
	    Cheater,
	    L"Possible cheating detected (%s) by "
	    "%s(%s)(%s) [%s %s]\n",
	    wscReason.c_str(), wscCharacterName, wscAccountDir.c_str(), wscAccountID.c_str(), wscHostName.c_str(),
	    wscIp.c_str());
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddKickLog(uint iClientID, std::wstring wscReason, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, wscReason);

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wscReason.c_str(), marker);

	const wchar_t* wszCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
	if (!wszCharname)
		wszCharname = L"";

	CAccount* acc = Players.FindAccountFromClientID(iClientID);
	std::wstring wscAccountDir;
	HkGetAccountDirName(acc, wscAccountDir);

	AddLog(Kick, L"Kick (%s): %s(%s)(%s)\n", wszBuf, wszCharname, wscAccountDir.c_str(), HkGetAccountID(acc).c_str());
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddConnectLog(uint iClientID, std::wstring wscReason, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, wscReason);

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wscReason.c_str(), marker);

	const wchar_t* wszCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
	if (!wszCharname)
		wszCharname = L"";

	CAccount* acc = Players.FindAccountFromClientID(iClientID);
	std::wstring wscAccountDir;
	HkGetAccountDirName(acc, wscAccountDir);

	AddLog(
	    Connects, L"Connect (%s): %s(%s)(%s)\n", wszBuf, wszCharname, wscAccountDir.c_str(),
	    HkGetAccountID(acc).c_str());
	return true;
}