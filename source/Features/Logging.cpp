#include "Global.hpp"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/spdlog.h"

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
		CheaterLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_cheaters", "flhook_logs/flhook_cheaters.log");
		KickLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_kicks", "flhook_logs/flhook_kicks.log");
		ConnectsLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_connects", "flhook_logs/flhook_connects.log");
		AdminCmdsLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_admincmds", "flhook_logs/flhook_admincmds.log");
		SocketCmdsLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_socketcmds", "flhook_logs/flhook_socketcmds.log");
		UserCmdsLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_usercmds", "flhook_logs/flhook_usercmds.log");
		PerfTimersLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_perftimers", "flhook_logs/flhook_perftimers.log");

		spdlog::flush_on(spdlog::level::err);
		spdlog::flush_every(std::chrono::seconds(3));

		if (IsDebuggerPresent())
		{
			WinDebugLog = spdlog::create_async<spdlog::sinks::msvc_sink_mt>("windows_debug");
			WinDebugLog->set_level(spdlog::level::debug);
		}

		if (FLHookConfig::c()->general.debugMode)
		{
			char szDate[64];
			time_t tNow = time(nullptr);
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

void AddLog(LogType LogType, LogLevel lvl, std::wstring wStr, ...)
{
	auto level = static_cast<spdlog::level::level_enum>(lvl);
	va_list marker;
	va_start(marker, wStr);

	wchar_t wszBuf[1024 * 8] = L"";

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wStr.c_str(), marker);
	std::string scText = wstos(wszBuf);

	switch (LogType)
	{
		case LogType::Cheater:
			CheaterLog->log(level, scText);
			break;
		case LogType::Kick:
			KickLog->log(level, scText);
			break;
		case LogType::Connects:
			ConnectsLog->log(level, scText);
			break;
		case LogType::AdminCmds:
			AdminCmdsLog->log(level, scText);
			break;
		case LogType::UserLogCmds:
			UserCmdsLog->log(level, scText);
			break;
		case LogType::SocketCmds:
			SocketCmdsLog->log(level, scText);
			break;
		case LogType::PerfTimers:
			PerfTimersLog->log(level, scText);
			break;
		case LogType::Normal:
			switch (level)
			{
				case spdlog::level::debug:
					Console::ConDebug(scText);
					break;
				case spdlog::level::info:
					Console::ConInfo(scText);
					break;
				case spdlog::level::warn:
					Console::ConWarn(scText);
					break;
				case spdlog::level::critical:
				case spdlog::level::err:
					Console::ConErr(scText);
					break;
				default: ;
			}
			
			FLHookLog->log(level, scText);
			break;
	}

	if (lvl == LogLevel::Debug && FLHookDebugLog)
	{
		FLHookDebugLog->debug(scText);
	}

	if (IsDebuggerPresent())
	{
		WinDebugLog->debug(scText);
	}

	if (lvl == LogLevel::Critical)
	{
		// Ensure all is flushed!
		spdlog::shutdown();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandleCheater(ClientId client, bool bBan, std::wstring wscReason, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, wscReason);

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wscReason.c_str(), marker);

	AddCheaterLog(client, wszBuf);

	if (wscReason[0] != '#' && Players.GetActiveCharacterName(client))
	{
		std::wstring character = (wchar_t*)Players.GetActiveCharacterName(client);

		wchar_t wszBuf2[500];
		swprintf_s(wszBuf2, L"Possible cheating detected: %s", character.c_str());
		Hk::Message::MsgU(wszBuf2);
	}

	if (bBan)
		Hk::Player::Ban(client, true);
	if (wscReason[0] != '#')
		Hk::Player::Kick(client);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AddCheaterLog(const std::variant<uint, std::wstring>& player, const std::wstring& wscReason)
{
	ClientId client = Hk::Client::ExtractClientID(player);

	CAccount* acc = Players.FindAccountFromClientID(client);
	std::wstring wscAccountDir = L"???";
	std::wstring wscAccountId = L"???";
	if (acc)
	{
		wscAccountDir = Hk::Client::GetAccountDirName(acc);
		wscAccountId = Hk::Client::GetAccountID(acc).value();
	}

	std::wstring wscHostName = L"???";
	std::wstring wscIp = L"???";

	wscHostName = ClientInfo[client].wscHostname;
	wscIp = Hk::Admin::GetPlayerIP(client);

	const auto wscCharacterName = Players.GetActiveCharacterName(client);

	AddLog(LogType::Cheater, LogLevel::Info, L"Possible cheating detected (%s) by %s(%s)(%s) [%s %s]", 
		wscReason.c_str(), wscCharacterName, wscAccountDir.c_str(), wscAccountId.c_str(), wscHostName.c_str(), wscIp.c_str());
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AddKickLog(ClientId client, std::wstring wscReason, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, wscReason);

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wscReason.c_str(), marker);

	const wchar_t* wszCharname = (wchar_t*)Players.GetActiveCharacterName(client);
	if (!wszCharname)
		wszCharname = L"";

	CAccount* acc = Players.FindAccountFromClientID(client);
	std::wstring wscAccountDir = Hk::Client::GetAccountDirName(acc);

	AddLog(LogType::Kick, LogLevel::Info, L"Kick (%s): %s(%s)(%s)\n", wszBuf, wszCharname, wscAccountDir.c_str(), Hk::Client::GetAccountID(acc).value().c_str());
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AddConnectLog(ClientId client, std::wstring wscReason, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, wscReason);

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wscReason.c_str(), marker);

	const wchar_t* wszCharname = (wchar_t*)Players.GetActiveCharacterName(client);
	if (!wszCharname)
		wszCharname = L"";

	CAccount* acc = Players.FindAccountFromClientID(client);
	std::wstring wscAccountDir = Hk::Client::GetAccountDirName(acc);

	AddLog(LogType::Connects, LogLevel::Info, L"Connect (%s): %s(%s)(%s)\n", wszBuf, wszCharname, wscAccountDir.c_str(), Hk::Client::GetAccountID(acc).value().c_str());
	return true;
}