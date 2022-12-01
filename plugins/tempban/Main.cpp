#include "Main.h"

namespace Plugins::Tempban
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/**************************************************************************************************************
	Check if TempBans exceeded
	**************************************************************************************************************/

	void TimerCheckKick()
	{
		// timed out tempbans get deleted here

		global->returncode = ReturnCode::Default;

		for (auto it = global->TempBans.begin(); it != global->TempBans.end(); ++it)
		{
			if (((*it).banStart + (*it).banDuration) < timeInMS())
			{
				global->TempBans.erase(it);
				break; // fix to not overflow the list
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Error __stdcall TempBan(const std::wstring& wscCharname, uint _duration)
	{
		const uint iClientID = GetClientIdFromCharname(wscCharname);

		mstime duration = 1000 * _duration * 60;
		TempbanInfo tempban;
		tempban.banStart = timeInMS();
		tempban.banDuration = duration;

		CAccount* acc;
		if (iClientID != -1)
			acc = Players.FindAccountFromClientID(iClientID);
		else
		{
			if (!(acc = GetAccountByCharname(wscCharname)))
				return CharDoesNotExist;
		}
		std::wstring wscID = GetAccountID(acc);

		tempban.accountId = wscID;
		global->TempBans.push_back(tempban);

		if (iClientID != -1 && Kick(iClientID) != E_OK)
		{
			AddLog(LogType::Kick, LogLevel::Info, wscCharname + L" could not be kicked (TempBan Plugin)");
			Console::ConInfo(wscCharname + L" could not be kicked (TempBan Plugin)");
		}

		return E_OK;
	}

	bool TempBannedCheck(uint iClientID)
	{
		CAccount* acc;
		acc = Players.FindAccountFromClientID(iClientID);

		std::wstring wscID = GetAccountID(acc);

		for (auto& ban : global->TempBans)
		{
			if (ban.accountId == wscID)
				return true;
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void __stdcall Login(struct SLoginInfo const& li [[maybe_unused]], unsigned int& iClientID)
	{
		global->returncode = ReturnCode::Default;

		// check for tempban
		if (TempBannedCheck(iClientID))
		{
			global->returncode = ReturnCode::SkipAll;
			Kick(iClientID);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CmdTempBan(CCmds* classptr, const std::wstring& wscCharname, uint iDuration)
	{
		// right check
		if (!(classptr->rights & RIGHT_KICKBAN))
		{
			classptr->Print(L"ERR No permission");
			return;
		}

		if ((classptr->LastErr = TempBan(wscCharname, iDuration)) == E_OK) // success
			classptr->Print(L"OK");
		else
			classptr->PrintError();
	}

	bool ExecuteCommandString(CCmds* classptr, const std::wstring& wscCmd)
	{
		if (wscCmd == L"tempban")
		{
			global->returncode = ReturnCode::SkipAll; // do not let other plugins kick in
			                                          // since we now handle the command

			CmdTempBan(classptr, classptr->ArgCharname(1), classptr->ArgInt(2));

			return true;
		}

		return false;
	}

	void CmdHelp(CCmds* classptr) { classptr->Print(L"tempban <charname>"); }

	TempBanCommunicator::TempBanCommunicator(std::string plug) : PluginCommunicator(plug) { this->TempBan = TempBan; }
} // namespace Plugins::Tempban

using namespace Plugins::Tempban;

DefaultDllMain()

    extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(TempBanCommunicator::pluginName);
	pi->shortName("tempban");
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &TimerCheckKick);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &Login, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Help, &CmdHelp);

	// Register IPC
	global->communicator = new TempBanCommunicator(TempBanCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
}