#include "Global.hpp"
#define SPDLOG_USE_STD_FORMAT
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
		FLHookLog = spdlog::basic_logger_mt<spdlog::async_factory>("FLHook", "logs/FLHook.log");
		CheaterLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_cheaters", "logs/flhook_cheaters.log");
		KickLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_kicks", "logs/flhook_kicks.log");
		ConnectsLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_connects", "logs/flhook_connects.log");
		AdminCmdsLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_admincmds", "logs/flhook_admincmds.log");
		SocketCmdsLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_socketcmds", "logs/flhook_socketcmds.log");
		UserCmdsLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_usercmds", "logs/flhook_usercmds.log");
		PerfTimersLog = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_perftimers", "logs/flhook_perftimers.log");

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

			std::string sDebugLog = "./logs/debug/FLHookDebug_" + (std::string)szDate;
			sDebugLog += ".log";

			FLHookDebugLog = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", sDebugLog);
			FLHookDebugLog->set_level(spdlog::level::debug);
		}
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		Console::ConErr(std::format("Log initialization failed: {}", ex.what()));
		return false;
	}
	return true;
}

void AddLog(LogType LogType, LogLevel lvl, const std::string& str)
{
	auto level = static_cast<spdlog::level::level_enum>(lvl);

	switch (LogType)
	{
		case LogType::Cheater:
			CheaterLog->log(level, str);
			break;
		case LogType::Kick:
			KickLog->log(level, str);
			break;
		case LogType::Connects:
			ConnectsLog->log(level, str);
			break;
		case LogType::AdminCmds:
			AdminCmdsLog->log(level, str);
			break;
		case LogType::UserLogCmds:
			UserCmdsLog->log(level, str);
			break;
		case LogType::SocketCmds:
			SocketCmdsLog->log(level, str);
			break;
		case LogType::PerfTimers:
			PerfTimersLog->log(level, str);
			break;
		case LogType::Normal:
			switch (level)
			{
				case spdlog::level::debug:
					Console::ConDebug(str);
					break;
				case spdlog::level::info:
					Console::ConInfo(str);
					break;
				case spdlog::level::warn:
					Console::ConWarn(str);
					break;
				case spdlog::level::critical:
				case spdlog::level::err:
					Console::ConErr(str);
					break;
				default: ;
			}
			
			FLHookLog->log(level, str);
			break;
		default:
			break;
	}

	if (lvl == LogLevel::Debug && FLHookDebugLog)
	{
		FLHookDebugLog->debug(str);
	}

	if (IsDebuggerPresent() && WinDebugLog)
	{
		WinDebugLog->debug(str);
	}

	if (lvl == LogLevel::Critical)
	{
		// Ensure all is flushed!
		spdlog::shutdown();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandleCheater(ClientId client, bool bBan, std::string reason)
{
	AddCheaterLog(client, reason);

	if (reason[0] != '#' && Players.GetActiveCharacterName(client))
	{
		std::wstring character = (wchar_t*)Players.GetActiveCharacterName(client);
		Hk::Message::MsgU(std::format(L"Possible cheating detected: {}", character.c_str()));
	}

	if (bBan)
		Hk::Player::Ban(client, true);
	if (reason[0] != '#')
		Hk::Player::Kick(client);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AddCheaterLog(const std::variant<uint, std::wstring>& player, const std::string& wscReason)
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

	const std::wstring wscCharacterName = Hk::Client::GetCharacterNameByID(client).value();

	AddLog(LogType::Cheater, LogLevel::Info, wstos(std::format(L"Possible cheating detected ({}) by {}({})({}) [{} {}]", 
		stows(wscReason), wscCharacterName, wscAccountDir.c_str(), wscAccountId.c_str(), wscHostName.c_str(), wscIp.c_str())));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AddKickLog(ClientId client, const std::string& wscReason)
{
	const wchar_t* wszCharname = (wchar_t*)Players.GetActiveCharacterName(client);
	if (!wszCharname)
		wszCharname = L"";

	CAccount* acc = Players.FindAccountFromClientID(client);
	std::wstring wscAccountDir = Hk::Client::GetAccountDirName(acc);

	AddLog(LogType::Kick, LogLevel::Info, wstos(std::format(L"Kick ({}): {}({})({})\n", stows(wscReason), wszCharname, wscAccountDir.c_str(), Hk::Client::GetAccountID(acc).value().c_str())));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AddConnectLog(ClientId client, const std::string& wscReason)
{
	const wchar_t* wszCharname = (wchar_t*)Players.GetActiveCharacterName(client);
	if (!wszCharname)
		wszCharname = L"";

	CAccount* acc = Players.FindAccountFromClientID(client);
	std::wstring wscAccountDir = Hk::Client::GetAccountDirName(acc);

	AddLog(LogType::Connects, LogLevel::Info, wstos(std::format(L"Connect ({}): {}({})({})\n", stows(wscReason), wszCharname, wscAccountDir.c_str(), Hk::Client::GetAccountID(acc).value().c_str())));
	return true;
}